#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/database/htdb_slab.hpp>
#include <bitcoin/utility/timed_section.hpp>
using namespace libbitcoin;
using namespace libbitcoin::chain;

constexpr size_t total_txs = 20000;
constexpr size_t tx_size = 200;
constexpr size_t buckets = 100;

data_chunk generate_random_bytes(
    std::default_random_engine& engine, size_t size)
{
    data_chunk result(size);
    for (uint8_t& byte: result)
        byte = engine() % std::numeric_limits<uint8_t>::max();
    return result;
}

void write_data()
{
    touch_file("htdb_slabs");
    mmfile file("htdb_slabs");
    BITCOIN_ASSERT(file.data());
    file.resize(4 + 8 * buckets + 8 + total_txs * tx_size * 2);

    htdb_slab_header header(file, 0);
    header.initialize_new(buckets);
    header.start();

    slab_allocator alloc(file, 4 + 8 * buckets);
    alloc.initialize_new();
    alloc.start();

    htdb_slab<hash_digest> ht(header, alloc);

    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);
        auto write = [&value](uint8_t* data)
        {
            std::copy(value.begin(), value.end(), data);
        };
        ht.store(key, value.size(), write);
    }
}

void validate_data()
{
    mmfile file("htdb_slabs");
    BITCOIN_ASSERT(file.data());

    htdb_slab_header header(file, 0);
    header.start();

    BITCOIN_ASSERT(header.size() == buckets);

    slab_allocator alloc(file, 4 + 8 * header.size());
    alloc.start();

    htdb_slab<hash_digest> ht(header, alloc);

    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);

        const slab_type slab = ht.get(key);
        BITCOIN_ASSERT(slab);

        BITCOIN_ASSERT(std::equal(value.begin(), value.end(), slab));
    }
}

void read_data()
{
    mmfile file("htdb_slabs");
    BITCOIN_ASSERT(file.data());

    htdb_slab_header header(file, 0);
    header.start();

    BITCOIN_ASSERT(header.size() == buckets);

    slab_allocator alloc(file, 4 + 8 * header.size());
    alloc.start();

    htdb_slab<hash_digest> ht(header, alloc);

    std::ostringstream oss;
    oss << "txs = " << total_txs << " size = " << tx_size
        << " buckets = " << buckets << " |  ";

    timed_section t("ht.get()", oss.str());
    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);

        const slab_type slab = ht.get(key);
    }
}

void show_usage()
{
    std::cerr << "Usage: htdb_bench [-w]" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc != 1 && argc != 2)
    {
        show_usage();
        return -1;
    }
    const std::string arg = (argc == 2) ? argv[1] : "";
    if (arg == "-h" || arg == "--help")
    {
        show_usage();
        return 0;
    }
    if (arg == "-w" || arg == "--write")
    {
        std::cout << "Writing..." << std::endl;
        write_data();
        std::cout << "Validating..." << std::endl;
        validate_data();
        std::cout << "Done." << std::endl;
    }
    // Perform benchmark.
    read_data();
    return 0;
}

