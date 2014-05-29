#include <fstream>
#include <random>
#include <bitcoin/format.hpp>
#include <bitcoin/utility/assert.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <bitcoin/blockchain.hpp>
using namespace bc;
using namespace bc::blockchain;

void touch_file(const std::string& filename)
{
    std::ofstream outfile(filename);
    // Write byte so file is nonzero size.
    outfile.write("H", 1);
}

void create_new(const std::string& filename)
{
    touch_file("shard");
    mmfile file("shard");
    BITCOIN_ASSERT(file.data());
    hdb_shard_settings settings;
    hdb_shard shard(file, settings);
    shard.initialize_new();
}

data_chunk generate_random_bytes(size_t size)
{
    std::random_device random;
    std::default_random_engine engine(random());
    data_chunk result(size);
    for (uint8_t& byte: result)
        byte = engine() % std::numeric_limits<uint8_t>::max();
    return result;
}

void write_random_rows(hdb_shard& shard,
    hdb_shard_settings& settings, size_t count)
{
    // Just write the same data because generating random data is costly.
    data_chunk key = generate_random_bytes(settings.total_key_size);
    data_chunk value = generate_random_bytes(settings.row_value_size);
    const size_t scan_bitsize =
        settings.total_key_size * 8 - settings.sharded_bitsize;
    address_bitset scan_key(scan_bitsize);
    boost::from_block_range(key.begin(), key.end(), scan_key);
    const size_t scan_size = (scan_bitsize - 1) / 8 + 1;
    BITCOIN_ASSERT(scan_key.num_blocks() == scan_size);

    for (size_t i = 0; i < 300000; ++i)
    {
        shard.add(scan_key, value);
        shard.add(scan_key, value);
        shard.add(scan_key, value);
        shard.sync(i);
    }
}

int main()
{
    //create_new("shard");
    mmfile file("shard");
    BITCOIN_ASSERT(file.data());
    hdb_shard_settings settings;
    hdb_shard shard(file, settings);
    shard.start();

    //for (size_t i = 0; i < 1000; ++i)
    {
        //std::cout << i << std::endl;
        //write_random_rows(shard, settings, 6000);
        //shard.sync(i);
    }

    size_t i = 0;
    auto read_row = [&](const uint8_t* row)
    {
        ++i;
        data_chunk data(row, row + settings.row_value_size);
        if (i % 10000 == 0)
            std::cout << data << std::endl;
    };
    timed_section t("scan", "0111111");
    address_bitset key(std::string("0111111"));
    shard.scan(key, read_row, 0);
    std::cout << i << std::endl;

    // Delete from block 1 onwards
    //shard.unlink(1);
    return 0;
}

