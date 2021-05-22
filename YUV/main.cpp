#include "mat.h"
#include "types.h"

#include <cassert>
#include <iostream>
#include <fstream>

template<typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T))
{
    return is.read(reinterpret_cast<char*>(&num), size);
}

template <typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(value, max));
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
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
        mat<uint8_t> Y(h, w);
        for (size_t y{0}; y < h; y++){
            for (size_t x{0}; x < w; x++) {
                raw_read(is, dump_value);
                dump_value = clamp(dump_value, uint8_t{16}, uint8_t{235});
                Y(y, x) = dump_value;
            }
        }


        // read Cb values - we need to make sure the stream is at the next frame header for the next iteration
        mat<uint8_t> Cb(h, w);
        for (size_t y{0}; y < h / 2; y++){
            for (size_t x{0}; x < w / 2; x++) {
                raw_read(is, dump_value);
                dump_value = clamp(dump_value, uint8_t{16}, uint8_t{240});
                Cb(2*y, 2*x) = dump_value;
                Cb(2*y + 1, 2*x) = dump_value;
                Cb(2*y, 2*x + 1) = dump_value;
                Cb(2*y + 1, 2*x + 1) = dump_value;
            }
        }
        // same with Cr
        mat<uint8_t> Cr(h, w);
        for (size_t y{0}; y < h / 2; y++){
            for (size_t x{0}; x < w / 2; x++) {
                raw_read(is, dump_value);
                dump_value = clamp(dump_value, uint8_t{16}, uint8_t{240});
                Cr(2*y, 2*x) = dump_value;
                Cr(2*y + 1, 2*x) = dump_value;
                Cr(2*y, 2*x + 1) = dump_value;
                Cr(2*y + 1, 2*x + 1) = dump_value;
            }
        }

        mat<vec3b> frame(h, w);
        constexpr double RGB_MIN = 0;
        constexpr double RGB_MAX = 255;
        uint8_t r, g, b;
        for (size_t y{0}; y < h; y++){
            for (size_t x{0}; x < w; x++) {
                const auto modified_y = Y(y, x) - 16;
                const auto modified_cb = Cb(y, x) - 128;
                const auto modified_cr = Cr(y, x) - 128;
                r = static_cast<uint8_t>(clamp(1.164 * modified_y + 0.000 * modified_cb + 1.596 * modified_cr, RGB_MIN, RGB_MAX));
                g = static_cast<uint8_t>(clamp(1.164 * modified_y - 0.392 * modified_cb - 0.813 * modified_cr, RGB_MIN, RGB_MAX));
                b = static_cast<uint8_t>(clamp(1.164 * modified_y + 2.017 * modified_cb + 0.000 * modified_cr, RGB_MIN, RGB_MAX));
                frame(y, x) = {r, g, b};
            }
        }
        frames.push_back(frame);
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Syntax: " << argv[0] << " <filename>" << std::endl;
        std::exit(1);
    }

    std::vector<mat<vec3b>> frames;
    y4m_extract_color(argv[1], frames);
    return 0;
}
