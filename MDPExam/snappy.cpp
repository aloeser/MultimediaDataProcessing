#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>


template <typename T>
std::istream& raw_read(std::istream& is, T& num, bool allow_empty = false, size_t size = sizeof(T)) {
    is.read(reinterpret_cast<char*>(&num), size);
    if (!allow_empty) {
        assert(is);
    }
    return is;
}

template <typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&num), size);
}

size_t read_preamble(std::istream& is) {
    size_t result = 0;
    size_t bits_read = 0;

    std::vector<uint8_t> bytes;
    uint8_t byte;
    while (bits_read < 64) {
        raw_read(is, byte);
        uint8_t first_bit = 0b10000000 & byte;
        uint8_t value = 0b01111111 & byte;
        bytes.push_back(value);
        bits_read += 7;
        if (first_bit == 0) {
            break;
        }
    }

    for (int64_t i = bytes.size() - 1; i >= 0;i--) {
        result = result << 7;
        result += bytes[i];
    }

    return result;
}

size_t symbols_printed = 0;
void write_symbol(std::ostream& os, std::vector<uint8_t>& symbols, uint8_t value) {
    raw_write(os, value);
    symbols.push_back(value);
    symbols_printed++;
}

void copy_from(std::ostream& os, const size_t offset, const size_t length, std::vector<uint8_t>& symbols) {
    assert(offset <= symbols.size());
    const size_t start_pos = symbols.size() - offset;
    for (size_t i = 0; i < length; i++) {
        uint8_t new_value = symbols.at(start_pos + i);
        write_symbol(os, symbols, new_value);
    }
}

int main(int argc, char* argv[]) {
    assert(argc == 3);
    std::string in_file = argv[1];
    std::string out_file = argv[2];

    std::ifstream is(in_file, std::ios::binary);
    if (!is) {
        std::cerr << "cannot open file " << in_file << std::endl;
        return 1;
    }

    std::ofstream os(out_file, std::ios::binary);
    if (!os) {
        std::cerr << "cannot open file " << out_file << std::endl;
        return 1;
    }


    size_t uncompressed_file = read_preamble(is);

    std::vector<uint8_t> symbols;

    // read first literal
    uint8_t tag;
    while (raw_read(is, tag, true)) {
        uint8_t type = tag & 0b00000011;
        if (type == 0b00){
            // literal
            size_t length = (tag & 0b11111100) >> 2;
            if (length <= 59) { // because we actually consider length - 1
                // nothing to do
            } else {
                uint8_t num_bytes_for_length = length - 60 + 1;
                length = 0;
                for (size_t length_byte = 0; length_byte < num_bytes_for_length; length_byte++) {
                    uint8_t length_fragment;
                    raw_read(is, length_fragment);
                    size_t length_fragment_64 = length_fragment;
                    length += (length_fragment_64 << (length_byte * 8));
                }
            }
            length++;

            uint8_t literal_value;
            //std::cout << "writing literal: ";
            for (size_t i = 0; i < length; i++) {
                raw_read(is, literal_value);
                //std::cout << literal_value;
                write_symbol(os, symbols, literal_value);
            }
            //std::cout << std::endl;

        } else {
            // copy
            size_t length = 0;
            size_t offset = 0;
            if (type == 0b01) {
                // copy with one byte offset
                length = ((tag & 0b00011100) >> 2) + 4;
                offset = (tag & 0b11100000);
                offset = offset << 3;

                uint8_t extra_offset_byte;
                raw_read(is, extra_offset_byte);
                offset += extra_offset_byte;
            } else if (type == 0b10) {
                // copy two byte offset
                length = (tag & 0b11111100) >> 2;
                length++;
                uint16_t offset_16;
                raw_read(is, offset_16);  // should be LE
                offset = offset_16;
            } else if (type == 0b11) {
                // copy with four byte offset
                length = (tag & 0b11111100) >> 2;
                length++;
                uint32_t offset_32;
                raw_read(is, offset_32); // should be LE
                offset = offset_32;
            }
            copy_from(os, offset, length, symbols);
        }
    }

    assert(symbols_printed == uncompressed_file);

    //std::cout << "Hello, World!" << std::endl;
    return 0;
}
