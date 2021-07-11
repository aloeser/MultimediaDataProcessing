//
// Created by aloeser on 11.07.21.
//

#pragma once

#include "mat.h"
#include <vector>

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded);
std::string Base64Encode(const std::vector<uint8_t>& v);