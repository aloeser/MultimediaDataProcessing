#include <cassert>
#include <cctype>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

class BencodeElement {

public:
    //virtual void parse_from_stream(std::istream& is) = 0;
    //virtual void print_to_stream(std::ostream& os) = 0;
    virtual void print_to_stream(std::ostream& os, size_t indent = 0, bool apply_indent = true, bool final_newline = true) const = 0 ;
};

class BencodeInt : public BencodeElement {
    int64_t value;

public:
    BencodeInt(std::istream& is) {
        assert(is.peek() == 'i');
        is.get();
        is >> value;
        assert(is.peek() == 'e');
        is.get();
    }

    void print_to_stream(std::ostream& os, size_t indent = 0, bool apply_indent = true, bool final_newline = true) const override {
        if (apply_indent) {
            for (size_t i{0}; i < indent; i++) {
                os << "\t";
            }
        }
        os << value;
        if (final_newline) {
            os << "\n";
        }
    }
};

class BencodeString : public BencodeElement {
    std::string value;

public:
    BencodeString(std::istream& is) {
        const char peek = is.peek();
        assert(std::isdigit(peek));
        size_t length;
        is >> length;
        assert(is.peek() == ':');
        is.get();
        value = std::string(length, ' ');
        is.read(value.data(), length);
    }

    bool operator<(const BencodeString& rhs) const {
        return value  < rhs.value;
    }

    void print_to_stream(std::ostream& os, size_t indent = 0, bool apply_indent = true, bool final_newline = true) const override {
        if (apply_indent) {
            for (size_t i{0}; i < indent; i++) {
                os << "\t";
            }
        }

        os << "\"";
        for (int i{0}; i < value.length();i++) {
            if (value[i] < 32 || value[i] > 126) {
                os << '.';
            } else {
                os << value[i];
            }
        }

        os << "\"";
        if (final_newline) {
            os << "\n";
        }
    }

    bool is_pieces() const {
        return value == "pieces";
    }

    std::vector<std::string> get_piece_hashes() {
        assert(value.length() % 20 == 0);

        std::vector<std::string> hashes;
        std::stringstream ss;
        for (size_t i{0}; i < value.length(); i++) {
            uint8_t byte = value[i];
            ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(byte);
            std::string byte_string = ss.str();
            if (i % 20 == 19) {
                std::string s = ss.str();
                hashes.push_back(s);
                ss = std::stringstream{};
            }
        }
        return hashes;
    }
};

class BencodeList : public BencodeElement {
    std::vector<std::shared_ptr<BencodeElement>> elements;

public:
    BencodeList(std::istream& is);

    void print_to_stream(std::ostream &os, size_t indent = 0, bool apply_indent = true, bool final_newline = true) const override {
        if (apply_indent)  {
            for (size_t i{0}; i < indent; i++) {
                os << "\t";
            }
        }
        os << "[\n";

        for (const auto& element : elements) {
            element->print_to_stream(os, indent + 1);
        }

        for (size_t i{0}; i < indent; i++) {
            os << "\t";
        }

        os << "]";
        if (final_newline) {
            os << "\n";
        }
    }
};

class BencodeDictionary : public BencodeElement {
    std::map<BencodeString, std::shared_ptr<BencodeElement>> elements;


    void print_pieces(std::ostream& os, std::shared_ptr<BencodeElement> pieces) const {
        const auto pieces_string = std::dynamic_pointer_cast<BencodeString>(pieces);
        assert(pieces_string);

        os << "\t\t\"pieces\" => \n";
        const auto hashes = pieces_string->get_piece_hashes();
        for (const auto& hash : hashes) {
            os << "\t\t\t" << hash << "\n";
        }
    }

public:
    BencodeDictionary(std::istream& is) {
        assert(is.peek() == 'd');
        is.get();
        while (true) {
            const char first_peek = is.peek();
            if (first_peek == 'e') {
                is.get();
                break;
            }
            // assumption/question: dictionary keys are always strings?
            const auto key = BencodeString(is);

            const char peek = is.peek();
            std::shared_ptr<BencodeElement> value;
            if (peek == 'i') {
                value = std::make_shared<BencodeInt>(is);
            } else if (std::isdigit(peek)) {
                value = std::make_shared<BencodeString>(is);
            } else if (peek == 'l') {
                value = std::make_shared<BencodeList>(is);
            } else if (peek == 'd') {
                value = std::make_shared<BencodeDictionary>(is);
            } else {
                throw std::logic_error("Character '" + std::to_string(peek) + "' is not valid at this position");
            }
            elements[key] = value;
        }
    }

    void print_to_stream(std::ostream &os, size_t indent = 0, bool apply_indent = true, bool final_newline = true) const override {
        if (apply_indent)  {
            for (size_t i{0}; i < indent; i++) {
                os << "\t";
            }
        }
        os << "{\n";

        for (const auto& [key, value] : elements) {
            if (key.is_pieces()) {
                print_pieces(os, value);
            } else {
                key.print_to_stream(os, indent + 1, true, false);
                os << " => ";
                value->print_to_stream(os, indent + 1 , false, true);
            }
        }

        for (size_t i{0}; i < indent; i++) {
            os << "\t";
        }

        os << "}";
        if (final_newline) {
            os << "\n";
        }
    }
};

BencodeList::BencodeList(std::istream& is) {
    assert(is.peek() == 'l');
    is.get();
    while (true) {
        const char first_peek = is.peek();
        if (first_peek == 'e') {
            is.get();
            break;
        }

        const char peek = is.peek();
        std::shared_ptr<BencodeElement> value;
        if (peek == 'i') {
            value = std::make_shared<BencodeInt>(is);
        } else if (std::isdigit(peek)) {
            value = std::make_shared<BencodeString>(is);
        } else if (peek == 'l') {
            value = std::make_shared<BencodeList>(is);
        } else if (peek == 'd') {
            value = std::make_shared<BencodeDictionary>(is);
        } else {
            throw std::logic_error("Character '" + std::to_string(peek) + "' is not valid at this position");
        }
        elements.push_back(value);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.torrent>" << std::endl;
        std::exit(-1);
    }

    std::ifstream is{argv[1]};
    if (!is) {
        std::cerr << "Could not open file: " << argv[1] << std::endl;
        std::exit(-1);
    }

    const auto contents = BencodeDictionary(is);
    contents.print_to_stream(std::cout);
    return 0;
}
