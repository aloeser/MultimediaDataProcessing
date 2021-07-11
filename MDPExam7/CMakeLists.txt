cmake_minimum_required(VERSION 3.19)
project(MDPExam7)

set(CMAKE_CXX_STANDARD 20)

add_executable(MDPExam7 main.cpp ppm.cpp ppm.h mat.h process_ppm.cpp process_ppm.h compress.cpp compress.h)
add_executable(MDPExam7Json ppm.cpp ppm.h mat.h process_ppm.cpp process_ppm.h compress.cpp compress.h json.cpp)