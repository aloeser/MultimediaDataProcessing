#include "ppm.h"

#include <iostream>
#include <string>

template <typename T>
std::istream& raw_read(std::istream& is, T& num, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&num), size);
}

std::istream& operator>>(std::istream& is, vec3b& v)
{
    //return is >> v[0] >> v[1] >> v[2];
    //*
    raw_read(is, v[0]);
    raw_read(is, v[1]);
    raw_read(is, v[2]);
    return is;
     //*/
}