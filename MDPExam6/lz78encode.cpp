//
// Created by aloeser on 10.07.21.
//

//
// Created by aloeser on 10.07.21.
//

//#include "lz78encode.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cmath>

template <typename T>
std::ostream& raw_write(std::ostream& os, T& value, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&value), size);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size=sizeof(T)){
    return is.read(reinterpret_cast<char*>(&num), size);
}

class bitwriter {
    uint8_t _value;
    uint8_t _bits;
    std::ostream& _os;

    void write_bit(uint32_t bit) {
        assert(bit == 1 || bit == 0);
        _value = (_value << 1)  + bit;
        _bits++;
        if (_bits == 8) {
            raw_write(_os, _value);
            _value = 0;
            _bits = 0;
        }
    }

    void write_bits(uint32_t value, uint8_t num_bits) {
        for (int64_t bit = num_bits; bit > 0; bit--) {
            uint32_t bit_value = (value >> (bit - 1)) & 1;
            write_bit(bit_value);
        }
    }

public:
    bitwriter(std::ostream& os) : _value{0}, _bits{0}, _os{os} {

    }

    void operator()(uint32_t value, uint8_t num_bits) {
        write_bits(value, num_bits);
    }

    ~bitwriter() {
        flush();
    }

    void flush(bool use_zero = true) {
        uint32_t bit = use_zero ? 0 : 1;
        while (_bits > 0) {
            write_bit(bit);
        }
    }
};

size_t find_longest_match(const std::vector<std::string>& dictionary, const std::string& window, bool allow_complete_match) {
    size_t index_longest_match = 0;
    // maybe checking for the longest string isnt necessary, it might suffice to take the last one, but not sure
    size_t longest_length = 0;

    for (size_t dictionary_id = 0; dictionary_id < dictionary.size(); dictionary_id++) {
        const auto& entry = dictionary[dictionary_id];
        if (window.starts_with(entry) && entry.size() > longest_length) {
            if (!allow_complete_match && window == entry) {
                continue;
            }
            index_longest_match = dictionary_id + 1;
            longest_length = entry.size();
        }
    }

    return index_longest_match;
}

void write_index(bitwriter& bw, size_t index_longest_match, size_t dictionary_size) {
    size_t num_bits = std::ceil(std::log2(dictionary_size + 1));
    assert(index_longest_match < (1u << num_bits));
    bw(index_longest_match, num_bits);
}

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
    std::cout << "TEEEEEEEST" << std::endl;
    std::ifstream is(input_filename, std::ios::binary);
    if (!is) {
        std::cerr << "Could not open input file " << input_filename << std::endl;
        return false;
    }

    std::ofstream os(output_filename, std::ios::binary);
    if (!os) {
        std::cerr << "Could not open output file " << output_filename << std::endl;
        return false;
    }

    // read all the input
    std::cout << "starting reading" << std::endl;
    std::vector<uint8_t> bytes;
    uint8_t byte;
    while (raw_read(is, byte)) {
        bytes.push_back(byte);
    }
    std::cout << "done reading" << std::endl;
    std::vector<std::string> dictionary;
    const size_t MAX_DICTIONARY_LENGTH = (1 << maxbits) - 1;
    bitwriter bw(os);

    bw('L', 8);
    bw('Z', 8);
    bw('7', 8);
    bw('8', 8);
    bw(maxbits, 5);
    for (size_t position = 0; position < bytes.size();) {
        const auto end_position = std::min(bytes.cbegin() + position + MAX_DICTIONARY_LENGTH + 1, bytes.cend());
        const std::string window(bytes.cbegin() + position, end_position);
        size_t index_longest_match = find_longest_match(dictionary, window, end_position != bytes.cend());

        write_index(bw, index_longest_match, dictionary.size());

        size_t length_of_match = index_longest_match == 0 ? 0 : dictionary[index_longest_match - 1].size();
        uint8_t new_char = window[length_of_match];
        bw(new_char, 8);
        //std::cout << "(" << index_longest_match << "," << static_cast<uint16_t>(new_char) << ")" << std::endl;

        dictionary.push_back(window.substr(0, length_of_match + 1));
        if (dictionary.size() > MAX_DICTIONARY_LENGTH) {
            dictionary.clear();
        }

        position += length_of_match + 1;
    }

    /*
    std::cout << "Dictionary contents:" << std::endl;
    std::cout << "0: <empty, pseudo-entry>" << std::endl;
    for (size_t i = 0;i < dictionary.size();i++) {
        std::cout << i+1 << ": " << dictionary[i] << std::endl;
    }
     */

    return true;
}