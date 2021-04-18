#include <array>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <unordered_map>
#include <vector>
#include <queue>


// Frequency counter, bitreader, and bitwriter shamelessly copied from Prof. Grana's code
struct frequency_counter {
    std::array<size_t, 256> occurrencies;

    frequency_counter() : occurrencies{ 0 } {}

    void operator()(uint8_t val) {
        ++occurrencies[val];
    }

    const size_t& operator[](size_t pos) const {
        return occurrencies[pos];
    }
    size_t& operator[](size_t pos) {
        return occurrencies[pos];
    }

    double entropy() const {
        double tot = 0.0;
        for (const auto& x : occurrencies) {
            tot += x;
        }

        double H = 0.0;
        for (const auto& x : occurrencies) {
            if (x > 0) {
                double px = x / tot;
                H += px * log2(px);
            }
        }

        return -H;
    }

    size_t num_elements() const {
        size_t num_elements = 0;
        for (const auto frequency : occurrencies) {
            num_elements += frequency;
        }
        return num_elements;
    }
};

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T))
{
    return os.write(reinterpret_cast<const char*>(&num), size);
}
template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T))
{
    return is.read(reinterpret_cast<char*>(&num), size);
}

class bitwriter {
    std::ostream& os_;
    uint8_t buffer_;
    size_t nbits_;

    std::ostream& write_bit(uint32_t u) {
        // buffer_ = buffer_ * 2 + u % 2;
        buffer_ = (buffer_ << 1) | (u & 1);
        ++nbits_;
        if (nbits_ == 8) {
            raw_write(os_, buffer_);
            nbits_ = 0;
        }
        return os_;
    }

public:
    bitwriter(std::ostream& os) : os_(os), nbits_(0) {}

    ~bitwriter() {
        flush();
    }

    // 43210
    // 00111

    // Writes the n least significant bits of u, from the most significant to the least significant,
    // i.e. from bit #n-1 to bit #0
    std::ostream& write(uint32_t u, size_t n) {
        // Solution 1: for (size_t i = n - 1; i < n; --i) {
        // Solution 2: for (size_t i = n; i > 0;) {
        //	               --i;
        // Solution 3: while (n > 0) {
        // 	               n--;
        // Solution 4: while (n --> 0) {
        //                 write_bit(u >> n); // writes bit #n
        //             }
        for (size_t i = 0; i < n; ++i) {
            write_bit(u >> (n - 1 - i));
        }
        return os_;
    }

    std::ostream& operator()(uint32_t u, size_t n) {
        return write(u, n);
    }

    void flush(uint32_t u = 0) {
        while (nbits_ > 0) {
            write_bit(u);
        }
    }
};

class bitreader {
    std::istream& is_;
    uint8_t buffer_;
    size_t nbits_;

    uint32_t read_bit() {
        if (nbits_ == 0) {
            raw_read(is_, buffer_);
            nbits_ = 8;
        }
        --nbits_;
        return (buffer_ >> nbits_) & 1;
    }

public:
    bitreader(std::istream& is) : is_(is), nbits_(0) {}

    std::istream& read(uint32_t& u, size_t n) {
        u = 0;
        while (n --> 0) {
            u = (u << 1) | read_bit();
        }
        return is_;
    }
};




using HuffmanTable = std::unordered_map<uint8_t,  std::string>;

class HuffmanNode {
    double _p;
    std::string _code;

    // only has a meaning if the node is a leaf
    uint8_t _byte_value;

    // only set if the node is not a leaf
    std::shared_ptr<HuffmanNode> _left;
    std::shared_ptr<HuffmanNode> _right;

    // we do not support codes longer than 32 bit (consistent with the exercise's specification)
    void set_code_recursively(const std::string& code, HuffmanTable& huffman_table) {
        _code = code;
        if (_left) {
            // a node has two children, or none, never just one
            _left->set_code_recursively(_code + "0", huffman_table);
            _right->set_code_recursively(_code + "1", huffman_table);
        } else {
            // node has no children -> leaf -> insert code into huffman table
            huffman_table[_byte_value] = _code;
            std::cout << "Byte " << static_cast<size_t>(_byte_value) << " has a code of length " << _code.length() << ": " << _code << std::endl;
        }
    }

public:
    HuffmanNode(const double p, const uint8_t byte_value) : _p{p}, _byte_value{byte_value} {}
    HuffmanNode(const double p, std::shared_ptr<HuffmanNode> left,  std::shared_ptr<HuffmanNode> right) : _p{p}, _left{left}, _right{right} {}

    double p() const {
        return _p;
    }

    const std::string& code() const {
        return _code;
    }

    bool operator>(const HuffmanNode& other) const {
        return _p > other._p;
    }

    HuffmanTable get_huffman_table() {
        HuffmanTable huffman_table;
        set_code_recursively("", huffman_table);
        return huffman_table;
    }
};

bool is_little_endian_system() {
    uint16_t x = 1;
    uint8_t *p = reinterpret_cast<uint8_t*>(&x);
    bool little_endian = *p == 1;
    std::cout << "little endian system: " << little_endian << std::endl;
    return little_endian;
}

template <typename T>
void swap_endianness(T& integer) {
    constexpr size_t bytes = sizeof(T);
    uint8_t *begin = reinterpret_cast<uint8_t*>(&integer);
    uint8_t *end = reinterpret_cast<uint8_t*>(&integer) + bytes - 1;
    while (begin < end) {
        uint8_t tmp = *begin;
        *begin = *end;
        *end = tmp;
        begin++;
        end--;
    }
}

template<typename T>
T as_big_endian(T integer) {
    if (!is_little_endian_system()) {
        swap_endianness(integer);
    }
    return integer;
}

uint32_t binary_string_to_uint32(const std::string& str) {
    return std::bitset<32>(str).to_ulong();
}

std::string uint32_to_binary_string(const uint32_t byte_code, const uint8_t length) {
    const std::string huffman_code = std::bitset<32>(byte_code).to_string();
    return huffman_code.substr(huffman_code.length() - length);
}

void write_header(std::ostream& os, const HuffmanTable& table, bitwriter& bw) {
    const std::string magic_number = "HUFFMAN1";
    for (const auto c : magic_number) {
        raw_write(os, c);
    }

    uint8_t num_entries;
    if (table.size() == 256) {
        num_entries = 0;
    } else {
        num_entries = table.size();
    }
    raw_write(os, num_entries);

    for (const auto& [symbol, huffman_code] : table) {
        const auto length = huffman_code.length();
        const uint32_t byte_code = binary_string_to_uint32((huffman_code));
        bw.write(symbol, 8);
        bw.write(length, 5);
        bw.write(byte_code, length);
    }
}

void encode_huffman(std::istream& is, std::ostream& os) {
    frequency_counter fc;
    std::vector<uint8_t> values;
    uint8_t value;
    while (raw_read(is, value)) {
        fc(value);
        values.push_back(value);
    }

    const size_t num_elements = fc.num_elements();
    std::priority_queue<HuffmanNode, std::vector<HuffmanNode>, std::greater<>> nodes;
    for (size_t i = 0; i < 256; i++) {
        if (fc[i] > 0) {
            nodes.push(HuffmanNode{1.0 * fc[i] / num_elements, static_cast<uint8_t>(i)});
        }
    }

    while (nodes.size() > 1) {
        const auto& right = std::make_shared<HuffmanNode>(nodes.top());
        nodes.pop();
        const auto left = std::make_shared<HuffmanNode>(nodes.top());
        nodes.pop();

        nodes.push(HuffmanNode{left->p() + right->p(), left, right});
    }

    auto root_node = nodes.top();
    const auto huffman_table = root_node.get_huffman_table();

    bitwriter bw{os};
    write_header(os, huffman_table, bw);

    const uint32_t num_symbols = values.size(); // as_big_endian(values.size());
    // trap: do not manually encode the value in big endian
    // bitwriter implicitly writes all numbers in big endian, as we start with writing the most significant bit first
    bw.write(num_symbols, 32);
    for (const auto value : values) {
        const auto& huffman_code = huffman_table.at(value);
        const uint32_t byte_code = std::bitset<32>(huffman_code).to_ulong();
        bw.write(byte_code, huffman_code.length());
    }
    bw.flush();
}

void decode_huffman(std::istream& is, std::ostream& os) {
    const uint64_t huffman1 = 0x485546464d414e31;
    uint64_t format;
    raw_read(is, format);
    swap_endianness(format);
    if (format != huffman1) {
        std::cout << "error: file does not start with HUFFMAN1" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    uint8_t num_symbols_uint8;
    raw_read(is, num_symbols_uint8);
    size_t num_symbols = num_symbols_uint8 != 0 ? num_symbols_uint8 : 256;

    bitreader br{is};

    std::unordered_map<std::string, uint8_t> huffman_table;
    for (size_t symbol_index{0}; symbol_index < num_symbols; symbol_index++) {
        uint32_t symbol = 0, length = 0, code = 0;
        br.read(symbol, 8);
        br.read(length, 5);
        br.read(code, length);
        const std::string huffman_code = uint32_to_binary_string(code, length);
        huffman_table[huffman_code] = static_cast<uint8_t>(symbol);
    }

    uint32_t num_values = 0;
    br.read(num_values, 32);
    // second part of the trap: bitreader is inherently big endian too, dont explicitly call it
    //if (is_little_endian_system()) {
    //    swap_endianness(num_values);
    //}

    for (size_t values_read{0}; values_read < num_values; values_read++) {
        uint32_t new_bit = 0;
        uint32_t code = 0;
        uint8_t length = 0;
        std::string huffman_code;
        do  {
            br.read(new_bit, 1);
            code = (code << 1) | new_bit;
            length++;
            huffman_code = uint32_to_binary_string(code, length);
        } while (huffman_table.find(huffman_code) == huffman_table.end());

        const uint8_t symbol = huffman_table[huffman_code];
        raw_write(os, symbol);
    }
}

int main(int argc, char* argv[]) {

    if (argc != 4 || (argv[1][0] != 'c' && argv[1][0] != 'd')) {
        std::cout << "Usage: " << argv[0] << " [c|d] <input file> <output file>" << std::endl;
        return 1;
    }

    std::ifstream is{argv[2], std::ios::binary};
    if (!is) {
        std::cout << "could not read input file: " << argv[2] << std::endl;
        return 1;
    }

    std::ofstream os{argv[3], std::ios::binary};
    if (!os) {
        std::cout << "could not write file: " << argv[3] << std::endl;
        return 1;
    }

   if (argv[1][0] == 'c') {
       encode_huffman(is, os);
   } else {
       decode_huffman(is, os);
   }
}