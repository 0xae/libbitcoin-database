/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_BLOCKCHAIN_DATABASE_HPP
#define LIBBITCOIN_BLOCKCHAIN_DATABASE_HPP

#include <cstddef>
#include <boost/filesystem.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/database/block_database.hpp>
#include <bitcoin/blockchain/database/spend_database.hpp>
#include <bitcoin/blockchain/database/transaction_database.hpp>
#include <bitcoin/blockchain/database/history_database.hpp>
#include <bitcoin/blockchain/database/stealth_database.hpp>
#include <bitcoin/blockchain/settings.hpp>

namespace libbitcoin {
namespace blockchain {

class BCB_API database
{
public:
    class store
    {
    public:
        store(const boost::filesystem::path& prefix);
        bool touch_all() const;

        boost::filesystem::path blocks_lookup;
        boost::filesystem::path blocks_rows;
        boost::filesystem::path spends;
        boost::filesystem::path transactions;
        boost::filesystem::path history_lookup;
        boost::filesystem::path history_rows;
        boost::filesystem::path stealth_index;
        boost::filesystem::path stealth_rows;
    };

    /// Create a new blockchain with a given path prefix and default paths.
    static bool initialize(const boost::filesystem::path& prefix,
        const chain::block& genesis);
    static bool touch_file(const boost::filesystem::path& file);

    database(const settings& settings);

    /// Deprecated
    database(const store& paths, size_t history_height=0,
        size_t stealth_height=0);

    /// Deprecated
    database(const boost::filesystem::path& prefix, size_t history_height=0,
        size_t stealth_height=0);

    void create();
    void start();

    void push(const chain::block& block);
    chain::block pop();

    block_database blocks;
    spend_database spends;
    transaction_database transactions;
    history_database history;
    stealth_database stealth;

private:
    typedef chain::input::list inputs;
    typedef chain::output::list outputs;

    void push_inputs(const hash_digest& tx_hash, size_t block_height,
        const inputs& inputs);
    void push_outputs(const hash_digest& tx_hash, size_t block_height,
        const outputs& outputs);
    void push_stealth(const hash_digest& tx_hash, const outputs& outputs);
    void pop_inputs(const inputs& inputs);
    void pop_outputs(const outputs& outputs);

    const size_t history_height_;
    const size_t stealth_height_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
