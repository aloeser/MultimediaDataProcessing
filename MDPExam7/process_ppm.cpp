//
// Created by aloeser on 11.07.21.
//

//#include "process_ppm.h"

#include "mat.h"
#include "ppm.h"

#include <fstream>
#include <iostream>

template <typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&num), size);
}

void skip_comment(std::istream& is) {
    while (isspace(is.peek())) {
        is.get();
    }
    if (is.peek() == '#') {
        std::cout << "found a comment";
        while (is.peek() != '\n') {
            is.get();
        }
    }
}

bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
    std::ifstream is(filename);
    if (!is) {
        std::cerr << "Cannot open image file " << filename << std::endl;
        return false;
    }

    std::string format;
    is >> format;
    assert(format == "P6");

    // test.ppm contains a comment..
    skip_comment(is);

    uint32_t width, height, max_color_value;
    is >> width >> height >> max_color_value;
    assert(width > 0);
    assert(height > 0);
    img.resize(height,  width);

    assert(max_color_value > 0 && max_color_value < 65536);
    assert(max_color_value < 256); // otherwise we cannot use vec3b..

    // erase whitespace
    is.get();

    const auto stream_position = is.tellg();
    is.close();
    is.open(filename, std::ios::binary);
    is.seekg(stream_position);

    size_t col = 0;
    size_t row = 0;
    size_t pixels_read = 0;
    uint8_t r, g, b;
    while (raw_read(is, r)) {
        raw_read(is, g);
        raw_read(is, b);
        img(row, col) = {r, g, b};
        col++;
        if (col == width) {
            col = 0;
            row++;
        }
        pixels_read++;
    }
    assert(pixels_read == img.size());

    return true;
}

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
    const auto cols = img.cols();
    const auto rows = img.rows();

    img_r.resize(rows, cols);
    img_g.resize(rows, cols);
    img_b.resize(rows, cols);
    for (size_t col = 0; col < cols; col++) {
        for (size_t row = 0; row < rows; row++) {
            const auto& pixel = img(row, col);
            img_r(row, col) = pixel[0];
            img_g(row, col) = pixel[1];
            img_b(row, col) = pixel[2];
        }
    }
}