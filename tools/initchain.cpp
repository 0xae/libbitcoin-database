/*
Create a new blockchain database.
*/
#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain.hpp>
using namespace bc;
using namespace bc::chain;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "initchain: No directory specified." << std::endl;
        return 1;
    }
    const std::string prefix = argv[1];
    initialize_blockchain(prefix);
    // Add genesis block.
    db_paths paths(prefix);
    db_interface interface(paths, {0});
    interface.start();
    const block_type genesis = genesis_block();
    interface.push(genesis);
    return 0;
}

