#include "process_ppm.h"
#include "compress.h"
#include "mat.h"
#include <cassert>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    assert(argc == 3);
    std::string filename = argv[1];
    mat<vec3b> img;
    LoadPPM(filename, img);

    std::vector<mat<uint8_t >> planes(3);
    SplitRGB(img, planes[0], planes[1], planes[2]);

    std::vector<std::vector<uint8_t>> packbits_encoded;
    for (size_t plane = 0; plane < planes.size(); plane++) {
        std::vector<uint8_t> packbit_codes;
        PackBitsEncode(planes[plane], packbit_codes);
        packbits_encoded.push_back(packbit_codes);
    }

    std::string json = "{\n";
    json += "\"width\": " + std::to_string(img.cols()) + ",\n";
    json += "\"rows\": " + std::to_string(img.rows()) + ",\n";
    json += "\"red\": \"" + Base64Encode(packbits_encoded[0]) + "\",\n";
    json += "\"green\": \"" + Base64Encode(packbits_encoded[1]) + "\",\n";
    json += "\"blue\": \"" + Base64Encode(packbits_encoded[2]) + "\",\n";
    json += "}";

    std::cout << json << std::endl;

    return 0;
}
