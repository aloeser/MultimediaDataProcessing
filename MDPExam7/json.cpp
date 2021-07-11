//
// Created by aloeser on 11.07.21.
//

#include "mat.h"
#include "ppm.h"

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <vector>
#include <iostream>

// we need to forward declare the functions from the other files, as OLJ doesnt accept additional headers,
// and I cannot copy their implementation without causing linker complaints because of duplicated implementations,
// so I assume that the other cpp files are linked too.. unfortunately there is no hint of this happening...
bool LoadPPM(const std::string& filename, mat<vec3b>& img) ;
void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b);
void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) ;
std::string Base64Encode(const std::vector<uint8_t>& v);

std::string JSON(const std::string& filename) {
    mat<vec3b> img;
    bool loaded = LoadPPM(filename, img);
    if (!loaded) return "{}";

    std::vector<mat<uint8_t >> planes(3);
    SplitRGB(img, planes[0], planes[1], planes[2]);

    std::vector<std::vector<uint8_t>> packbits_encoded;
    for (size_t plane = 0; plane < planes.size(); plane++) {
        std::vector<uint8_t> packbit_codes;
        PackBitsEncode(planes[plane], packbit_codes);
        packbits_encoded.push_back(packbit_codes);
    }

    std::string json = "{\n";
    json += "\t\"width\": " + std::to_string(img.cols()) + ",\n";
    json += "\t\"rows\": " + std::to_string(img.rows()) + ",\n";
    json += "\t\"red\": " + Base64Encode(packbits_encoded[0]) + "\",\n";
    json += "\t\"green\": " + Base64Encode(packbits_encoded[1]) +  "\",\n";
    json += "\t\"blue\": " + Base64Encode(packbits_encoded[2]) + "\",\n";
    json += "}";
    return json;
}

/*
int main(int argc, char* argv[]){
    assert(argc == 2);
    std::string filename = argv[1];
    std::cout << JSON(filename) << std::endl;
}
//*/