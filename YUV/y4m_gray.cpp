#include "mat.h"

#include <cassert>
#include <iostream>
#include <fstream>

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T))
{
    return is.read(reinterpret_cast<char*>(&num), size);
}

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
    std::ifstream is{filename};
    if (!is) {
        std::cerr << "could not open file: " << filename << std::endl;
        return false;
    }

    std::string magic_number(9, ' ');
    if (!is.read(magic_number.data(), 9)) {
        return false;
    }

    if (magic_number != "YUV4MPEG2") {
        std::cerr << "magic number is wrong: " << magic_number << std::endl;
        return false;
    }

    size_t w = -1, h = -1;
    // read headers
    while (is) {
        if (is.peek() == '\n') {
            is.get();
            break;
        }

        // header tag always starts with space
        assert(is.peek() == ' ');
        is.get();

        const char tag = is.get();
        std::string content;
        switch (tag) {
            case 'H':
                is >> h;
                break;
            case 'W':
                is >> w;
                break;
            case 'C':
            case 'I':
            case 'F':
            case 'A':
            case 'X':
                // just ignore the content
                is >> content;
                break;

            default:
                std::cerr << "invalid tag: " << std::to_string(tag);
                return false;
        }
    }
    // make sure that h and w have been read
    assert(h > 0 && w > 0 && h % 2 == 0 && w % 2 == 0);

    // start reading frames
    while (true) {
        std::string frame_magic_number(5, ' ');
        if (!is.read(frame_magic_number.data(), 5)) {
            if (is.gcount() == 0) {
                // we are done, no more frames
                break;
            } else {
                // couldnt read frame header, problem
                std::cerr << "could read only " << is.gcount() << " characters: " << frame_magic_number << " when 5 (FRAME) where expected" << std::endl;
            }

        }
        assert(frame_magic_number == "FRAME");

        // read header fields
        while (true) {
            if (is.peek() == '\n') {
                is.get();
                break;
            }

            // header fields start with whitespace
            assert(is.peek() == ' ');
            is.get();
            const char tag = is.get();
            std::string content;
            switch (tag) {
                case 'I':
                case 'X':
                    // valid tag, but we do not care about the content
                    is >> content;
                    break;

                default:
                    throw std::logic_error("invalid frame header tag: " + std::to_string(tag));
            }
        }


        // read Y values
        uint8_t dump_value;
        mat<uint8_t> frame(h, w);
        for (size_t y{0}; y < h; y++){
            for (size_t x{0}; x < w; x++) {
                raw_read(is, dump_value);
                frame(y, x) = dump_value;
            }
        }
        frames.push_back(frame);

        // read Cb values - we need to make sure the stream is at the next frame header for the next iteration
        for (size_t y{0}; y < h / 2; y++){
            for (size_t x{0}; x < w / 2; x++) {
                raw_read(is, dump_value);
            }
        }
        // same with Cr
        for (size_t y{0}; y < h / 2; y++){
            for (size_t x{0}; x < w / 2; x++) {
                raw_read(is, dump_value);
            }
        }

        if (is) {
            //std::string tmp(5, ' ');
            //is.read(tmp.data(), 5);
            //std::cout << "data is " << tmp;
            //const char c = is.peek();
            //const char c2 = c +1;
        } else {
            break;
        }

    }

    return true;
}

/*
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Syntax: " << argv[0] << " <filename>" << std::endl;
        std::exit(1);
    }

    std::vector<mat<uint8_t>> frames;
    y4m_extract_gray(argv[1], frames);
    return 0;
}
*/
