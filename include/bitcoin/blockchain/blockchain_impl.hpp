/*
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_BLOCKCHAIN_BLOCKCHAIN_IMPL_HPP
#define LIBBITCOIN_BLOCKCHAIN_BLOCKCHAIN_IMPL_HPP

#include <atomic>
#include <boost/interprocess/sync/file_lock.hpp>
#include <leveldb/db.h>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/blockchain.hpp>
#include <bitcoin/blockchain/organizer.hpp>
#include <bitcoin/blockchain/database/stealth_database.hpp>
#include <bitcoin/utility/subscriber.hpp>
#include <bitcoin/threadpool.hpp>

namespace libbitcoin {
    namespace chain {

class blockchain_common;
typedef std::shared_ptr<blockchain_common> blockchain_common_ptr;

class blockchain_impl
  : public blockchain
{
public:
    // Used by internal components so need public definition here
    typedef subscriber<
        const std::error_code&, size_t, const block_list&, const block_list&>
            reorganize_subscriber_type;

    typedef std::function<void (const std::error_code)> start_handler;

    BCB_API blockchain_impl(threadpool& pool);
    BCB_API ~blockchain_impl();

    // Non-copyable
    blockchain_impl(const blockchain_impl&) = delete;
    void operator=(const blockchain_impl&) = delete;

    BCB_API void start(const std::string& prefix, start_handler handle_start);
    BCB_API void stop();

    BCB_API void store(const block_type& block,
        store_block_handler handle_store);
    BCB_API void import(const block_type& block, size_t height,
        import_block_handler handle_import);

    // fetch block header by height
    BCB_API void fetch_block_header(size_t height,
        fetch_handler_block_header handle_fetch);
    // fetch block header by hash
    BCB_API void fetch_block_header(const hash_digest& block_hash,
        fetch_handler_block_header handle_fetch);
    // fetch transaction hashes in block by height
    BCB_API void fetch_block_transaction_hashes(size_t height,
        fetch_handler_block_transaction_hashes handle_fetch);
    // fetch transaction hashes in block by hash
    BCB_API void fetch_block_transaction_hashes(const hash_digest& block_hash,
        fetch_handler_block_transaction_hashes handle_fetch);
    // fetch height of block by hash
    BCB_API void fetch_block_height(const hash_digest& block_hash,
        fetch_handler_block_height handle_fetch);
    // fetch height of latest block
    BCB_API void fetch_last_height(fetch_handler_last_height handle_fetch);
    // fetch transaction by hash
    BCB_API void fetch_transaction(const hash_digest& transaction_hash,
        fetch_handler_transaction handle_fetch);
    // fetch height and offset within block of transaction by hash
    BCB_API void fetch_transaction_index(const hash_digest& transaction_hash,
        fetch_handler_transaction_index handle_fetch);
    // fetch spend of an output point
    BCB_API void fetch_spend(const output_point& outpoint,
        fetch_handler_spend handle_fetch);
    // fetch outputs, values and spends for an address.
    BCB_API void fetch_history(const payment_address& address,
        fetch_handler_history handle_fetch, size_t from_height=0);
    // fetch stealth results.
    BCB_API void fetch_stealth(const stealth_prefix& prefix,
        fetch_handler_stealth handle_fetch, size_t from_height=0);

    BCB_API void subscribe_reorganize(reorganize_handler handle_reorganize);

private:
    typedef std::atomic<size_t> seqlock_type;
    typedef std::unique_ptr<leveldb::DB> database_ptr;
    typedef std::unique_ptr<leveldb::Comparator> comparator_ptr;
    typedef std::unique_ptr<mmfile> mmfile_ptr;
    typedef std::unique_ptr<stealth_database> stealth_db_ptr;

    typedef std::function<bool (size_t)> perform_read_functor;

    bool initialize(const std::string& prefix);

    void begin_write();

    template <typename Handler, typename... Args>
    void finish_write(Handler handler, Args&&... args)
    {
        ++seqlock_;
        // seqlock is now even again.
        BITCOIN_ASSERT(seqlock_ % 2 == 0);
        handler(std::forward<Args>(args)...);
    }

    void do_store(const block_type& block,
        store_block_handler handle_store);
    void do_import(const block_type& block, size_t height,
        import_block_handler handle_import);

    // Uses sequence looks to try to read shared data.
    // Try to initiate asynchronous read operation. If it fails then
    // sleep for a small amount of time and then retry read operation.
    void fetch(perform_read_functor perform_read);

    template <typename Handler, typename... Args>
    bool finish_fetch(size_t slock, Handler handler, Args&&... args)
    {
        if (slock != seqlock_)
            return false;
        handler(std::forward<Args>(args)...);
        return true;
    }

    bool fetch_block_header_by_height(size_t height, 
        fetch_handler_block_header handle_fetch, size_t slock);
    bool fetch_block_header_by_hash(const hash_digest& block_hash, 
        fetch_handler_block_header handle_fetch, size_t slock);
    bool do_fetch_block_height(const hash_digest& block_hash,
        fetch_handler_block_height handle_fetch, size_t slock);
    bool do_fetch_last_height(
        fetch_handler_last_height handle_fetch, size_t slock);
    bool do_fetch_transaction(const hash_digest& transaction_hash,
        fetch_handler_transaction handle_fetch, size_t slock);
    bool do_fetch_transaction_index(const hash_digest& transaction_hash,
        fetch_handler_transaction_index handle_fetch, size_t slock);
    bool do_fetch_spend(const output_point& outpoint,
        fetch_handler_spend handle_fetch, size_t slock);
    bool do_fetch_history(const payment_address& address,
        fetch_handler_history handle_fetch, size_t from_height, size_t slock);
    bool do_fetch_stealth(const stealth_prefix& prefix,
        fetch_handler_stealth handle_fetch, size_t from_height, size_t slock);

    io_service& ios_;
    // Queue for writes to the blockchain.
    async_strand strand_;
    // Queue for serializing reorganization handler calls.
    async_strand reorg_strand_;
    // Lock the database directory with a file lock.
    boost::interprocess::file_lock flock_;
    // seqlock used for writes.
    seqlock_type seqlock_;

    // Comparator to order blocks by height logically.
    // Otherwise the last block in the database
    // might not be the largest height in our blockchain.
    comparator_ptr height_comparator_;
    leveldb::Options open_options_;

    // Blocks indexed by height.
    //   block height -> block header + list(tx_hashes)
    database_ptr db_block_;
    // Block heights indexed by hash (a secondary lookup table).
    //   block hash -> block height
    database_ptr db_block_hash_;
    // Transactions indexed by hash.
    //   tx hash -> tx height + tx index + tx
    database_ptr db_tx_;
    // Lookup whether an output point is spent.
    // Value is the input point spend.
    //   outpoint -> inpoint spend
    database_ptr db_spend_;

    // Address to list of output points + values
    database_ptr db_credit_;
    // Address to list of spend input points.
    database_ptr db_debit_;

    // Custom databases.
    // Stealth database. See <bitcoin/database/stealth_database.hpp>
    // https://wiki.unsystem.net/index.php/DarkWallet/Stealth#Database_file_format
    mmfile_ptr stealth_file_;
    stealth_db_ptr db_stealth_;

    blockchain_common_ptr common_;
    // Organize stuff
    orphans_pool_ptr orphans_;
    simple_chain_ptr chain_;
    organizer_ptr organize_;

    reorganize_subscriber_type::ptr reorganize_subscriber_;
};

    } // namespace chain
} // namespace libbitcoin

#endif

