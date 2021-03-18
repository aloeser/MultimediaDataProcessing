#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

template <typename T>
class Bitpacker {

public:
    Bitpacker() : _state{0}, _state_bits_used{0}, _state_bits_max{8 * sizeof(_state)}, _num_unpacked_numbers {0} {}

    void add_value(const int64_t value, const size_t bits) {
        assert(bits > 0);
        T bitmask{0};
        // start with the most significant bit
        for (size_t bit{bits}; bit > 0; bit--) {
            bitmask = 1 << (bit - 1);

            _state = _state << 1; // Move state one bit left
            _state_bits_used++;
            if ((bitmask & value) != 0) {
                // bit at position bit is 1, so we append 1
                _state = _state | 1;
            }

            // check whether the buffer is full
            if (_state_bits_used == _state_bits_max) {
                _values.push_back(_state);
                _state = 0;
                _state_bits_used = 0;
            }
        }
    }

    std::vector<int64_t> get_values(const size_t bits = 8 * sizeof(T)) const {
        assert(bits > 1);
        /*
        // shortcut: if the vector stores only entire numbers, we can return it
        if (bits == 8 * sizeof(T)) {
            return _values;
        }
         */

        // default case: we have to reconstruct the values
        // TODO: if this has to be efficient, we should calculate the required memory and reserve/resize the vector
        std::vector<int64_t> result;
        int64_t current_value;

        size_t num_numbers_unpacked = 0;
        size_t bits_written = 0;
        bool first_bit_read = false;

        for (const T value : _values) {
            for (size_t value_bit = 8 * sizeof(T); value_bit > 0; value_bit--) {

                current_value = current_value << 1;
                bits_written++;
                int bitmask = 1 << (value_bit - 1);
                bool bit_was_one = (bitmask & value) != 0;
                if (!first_bit_read) {
                    first_bit_read = true;
                    if (bit_was_one) {
                        // 2er complement: first bit negative -> number negative
                        current_value = -1;
                    } else {
                        // first bit = 0 -> positive number
                        current_value = 0;
                    }
                }
                if (bit_was_one) {
                    // bit value_bit of value was one, so change the last bit to 1
                    current_value = current_value | 1;
                }

                if (bits_written == bits) {
                    result.push_back(current_value);
                    first_bit_read = false;
                    bits_written = 0;

                    num_numbers_unpacked++;
                    if (num_numbers_unpacked == _num_unpacked_numbers) {
                        goto done_unpacking;
                    }
                }
            }
        }

        done_unpacking:

        return result;
    }

    void flush() {
        if (_state_bits_used > 0) {
            _state = _state << (_state_bits_max - _state_bits_used);
            _values.push_back(_state);
            _state = 0;
            _state_bits_used = 0;
        }
    }

    void read_text(const std::string& filename, const size_t bits = 8 * sizeof(T)) {
        std::ifstream file(filename, std::ios_base::in);
        std::vector<int64_t> values;
        int64_t number;
        while (file >> number) {
            values.push_back(number);
        }
        pack_values(values, bits);
        file.close(); // should happen automatically once file goes out of scope?
    }

    void pack_values(const std::vector<int64_t>& values, const size_t bits = 8 * sizeof(T)) {
        _values.clear();
        for (const auto& value : values) {
            add_value(value, bits);
            _num_unpacked_numbers++;
        }
        flush();
        _num_unpacked_numbers = _values.size();
    }

    void write_text(const std::string& filename, size_t bits = 8 * sizeof(T)) {
        std::ofstream file(filename, std::ios_base::out);
        const auto values = get_values(bits);
        for (const auto value : values) {
            file << value << std::endl;
            std::cout << value << " written" << std::endl;
        }
        file.close();
    }

    void read_binary(const std::string& filename,  size_t num_numbers) {
        _values.clear();
        std::ifstream file(filename, std::ios::binary);
        char number;
        while (file.read(&number, 1)) {
            add_value(number, 8);
        }
        flush();
        file.close(); // should happen automatically once file goes out of scope?
        _num_unpacked_numbers = num_numbers;
    }

    void write_binary(const std::string& filename) {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        file.write((char*)&_values[0], _values.size() * sizeof(T));
        file.close();
    }

private:
    std::vector<T> _values;
    T _state;
    size_t _state_bits_used;
    size_t _num_unpacked_numbers;
    const size_t _state_bits_max;

};


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input file> <output file>" << std::endl;
        return 1;
    }

    const size_t BITS = 6;
    Bitpacker<int8_t> b;
    b.read_text(argv[1], BITS);
    //b.read_binary(argv[1]);
    b.write_text(argv[2], BITS);
    //b.write_binary(argv[2]);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
