#include "lz78encode.hpp"
#include <iostream>
#include <cassert>

int main(int argc, char* argv[]) {
    assert(argc == 4);
    const std::string input_filename = argv[1];
    const std::string output_filename = argv[2];
    const int maxbits = std::stoi(argv[3]);
    lz78encode(input_filename, output_filename, maxbits);
    return 0;
}
