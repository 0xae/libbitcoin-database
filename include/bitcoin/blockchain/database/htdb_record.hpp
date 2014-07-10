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
#ifndef LIBBITCOIN_BLOCKCHAIN_HTDB_RECORD_HPP
#define LIBBITCOIN_BLOCKCHAIN_HTDB_RECORD_HPP

#include <bitcoin/blockchain/database/disk_array.hpp>
#include <bitcoin/blockchain/database/record_allocator.hpp>
#include <bitcoin/blockchain/database/types.hpp>

namespace libbitcoin {
    namespace chain {

typedef disk_array<index_type, index_type> htdb_record_header;

/**
 * A hashtable mapping hashes to fixed sized values (records).
 * Uses a combination of the disk_array and record_allocator.
 *
 * The disk_array is basically a bucket list containing the start
 * value for the hashtable chain.
 *
 * The record_allocator is used to create linked chains. A header
 * containing the hash of the item, and the next value is stored
 * with each record.
 *
 *   [ HashType ]
 *   [ next:4   ]
 *   [ record   ]
 *
 * By using the record_allocator instead of slabs, we can have smaller
 * indexes avoiding reading/writing extra bytes to the file.
 * Using fixed size records is therefore faster.
 */
template <typename HashType>
class htdb_record
{
public:
    typedef std::function<void (uint8_t*)> write_function;

    htdb_record(htdb_record_header& header, record_allocator& allocator);

    /**
     * Store a value. The provided write() function must write
     * the correct number of bytes (record_size - hash_size - 4).
     */
    void store(const HashType& key, write_function write);

    /**
     * Return the record for a given hash.
     */
    const record_type get(const HashType& key) const;

    /**
     * Delete a key-value pair from the hashtable by unlinking the node.
     */
    void unlink(const HashType& key);

private:
    htdb_record_header& header_;
    record_allocator& allocator_;
};

    } // namespace chain
} // namespace libbitcoin

#include <bitcoin/blockchain/impl/htdb_record.ipp>

#endif

