//
// Created by aloeser on 11.07.21.
//

//#include "compress.h"
#include "mat.h"
#include <cassert>
#include <vector>

size_t detect_run_length(const uint8_t *pos, const uint8_t *end) {
    char current = *pos;
    size_t length = 0;
    pos++;
    bool run = false;
    while (length < 127 && pos < end && *pos == current) {
        length++;
        pos++;
        run = true;
    }

    if (run) {
        length++;
    }
    return length;
}

size_t detect_copy_length(const uint8_t *it, const uint8_t *end) {
    uint8_t current;
    uint8_t prev = *it;
    size_t copy_length = 1;
    it++;
    while (copy_length < 128 && it < end) {
        if (prev != *it) {
            copy_length++;
            prev = *it;
            it++;
        } else {
            copy_length = std::max(copy_length - 1, 1ul);
            break;
        }
    }

    assert(copy_length >= 1 && copy_length <= 128);

    return copy_length;
}

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
    auto it = img.data();
    const auto end = img.data() + (img.rows()*img.cols());

    size_t pixels_processed = 0;

    while (it != end) {
        assert(it < end);
        const size_t current_run = detect_run_length(it, end);
        assert(it + current_run <= end);
        if (current_run > 0) {
            const uint8_t L = static_cast<uint8_t>(257 - current_run);
            encoded.push_back(L);
            encoded.push_back(*it);
            it += current_run;
            pixels_processed += current_run;
        } else {
            const size_t copy_length = detect_copy_length(it, end);
            assert(it + copy_length <= end);
            const uint8_t L = copy_length - 1;
            encoded.push_back(L);
            for (size_t i = 0; i < copy_length; i++) {
                encoded.push_back(*it);
                it++;
                pixels_processed++;
            }
        }
    }
    assert(pixels_processed == img.size());

    encoded.push_back(128); // EOD
}


std::vector<uint8_t> get_base64(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    uint8_t base64_1 = (byte1 & 0xFC) >> 2 ; // first six bits of byte 1
    uint8_t base64_2 = ((byte1 & 0x03) << 4) + ((byte2 & 0xF0) >> 4); // last two bits of byte1 + first four bits of byte2
    uint8_t base64_3 = ((byte2 & 0x0F) << 2)  + ((byte3 & 0xC0) >> 6); // last four bits of byte 2  + first two of byte3
    uint8_t base64_4 = byte3 & 0x3F; // prune the first two bits of the last byte
    assert(base64_1  < 64);
    assert(base64_2  < 64);
    assert(base64_3  < 64);
    assert(base64_4  < 64);
    return {base64_1, base64_2, base64_3, base64_4};
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
    const std::string TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;

    for (size_t i = 0; i < v.size(); i += 3) {
        uint8_t byte1 = v[i], byte2, byte3;
        if (i + 1 >= v.size()) {
            byte2 =  128;
        } else {
            byte2 = v[i+1];
        }
        if (i + 2 >= v.size()) {
            byte3 = 128;
        } else {
            byte3 = v[i+2];
        }

        const auto base64_values = get_base64(byte1, byte2, byte3);
        for (const auto base64_value : base64_values) {
            result += TABLE[base64_value];
        }
    }

    return result;
}