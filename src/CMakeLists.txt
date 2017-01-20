cmake_minimum_required (VERSION 2.6)
project (ImpactSearcher CXX)
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
add_library(libImpactSearcher SHARED image_helper.cpp main.cpp thread_2_center.cpp math_helpers.cpp thread_3_average.cpp image_bmp.cpp filesystem.cpp thread_4_presort.cpp thread_1_IO.cpp thread_5_check.cpp)
add_executable(ImpactSearcher main_console_wrapper.cpp)
target_compile_definitions(libImpactSearcher PUBLIC BUILDING_LIB _UNICODE UNICODE)
