#include <iostream>
#include <boost/lexical_cast.hpp>
#include <bitcoin/database.hpp>

using namespace bc;
using namespace bc::database;

template <size_t N>
slab_byte_pointer get0(htdb_slab_header& header, slab_manager& alloc,
    const data_chunk& key_data)
{
    typedef byte_array<N> hash_type;
    slab_hash_table<hash_type> ht(header, alloc);
    hash_type key;
    BITCOIN_ASSERT(key.size() == key_data.size());
    std::copy(key_data.begin(), key_data.end(), key.begin());
    return ht.get(key);
}

int main(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        std::cerr
            << "Usage: read_htdb_slab_value FILENAME KEY VALUE_SIZE [OFFSET]"
            << std::endl;
        return 0;
    }
    const std::string filename = argv[1];

    data_chunk key_data;
    if (!decode_base16(key_data, argv[2]))
    {
        std::cerr << "key data is not valid" << std::endl;
        return -1;
    }

    const size_t value_size = boost::lexical_cast<size_t>(argv[3]);
    file_offset offset = 0;
    if (argc == 5)
        offset = boost::lexical_cast<file_offset>(argv[4]);

    memory_map file(filename);
    BITCOIN_ASSERT(file.data());

    htdb_slab_header header(file, offset);
    header.start();

    slab_manager alloc(file, offset + 4 + 8 * header.size());
    alloc.start();

    slab_byte_pointer slab = nullptr;
    if (key_data.size() == 32)
    {
        slab = get0<32>(header, alloc, key_data);
    }
    else if (key_data.size() == 4)
    {
        slab = get0<4>(header, alloc, key_data);
    }
    else
    {
        std::cerr << "read_htdb_slab_value: unsupported key size."
            << std::endl;
        return -1;
    }
    data_chunk data(slab, slab + value_size);
    std::cout << encode_base16(data) << std::endl;
    return 0;
}

