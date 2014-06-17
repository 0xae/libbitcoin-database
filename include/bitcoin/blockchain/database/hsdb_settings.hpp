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
#ifndef LIBBITCOIN_BLOCKCHAIN_HDB_SETTINGS_HPP
#define LIBBITCOIN_BLOCKCHAIN_HDB_SETTINGS_HPP

#include <functional>
#include <bitcoin/types.hpp>
#include <bitcoin/utility/mmfile.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/database/types.hpp>

namespace libbitcoin {
    namespace chain {

struct BCB_API hsdb_shard_settings
{
    size_t scan_bitsize() const;
    size_t scan_size() const;
    size_t number_buckets() const;

    size_t version = 1;
    size_t shard_max_entries = 1000000;
    size_t total_key_size = 20;
    size_t sharded_bitsize = 8;
    size_t bucket_bitsize = 8;
    size_t row_value_size = 49;
};

/**
  * Load the shard settings from the control file.
  */
BCB_API hsdb_shard_settings load_shard_settings(const mmfile& file);

/**
  * Save the shard settings in the control file.
  */
BCB_API void save_shard_settings(
    mmfile& file, const hsdb_shard_settings& settings);

    } // namespace chain
} // namespace libbitcoin

#endif

