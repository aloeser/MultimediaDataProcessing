//
// Created by aloeser on 11.07.21.
//

#pragma once
#include "ppm.h"
#include "mat.h"

bool LoadPPM(const std::string& filename, mat<vec3b>& img);
void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b);