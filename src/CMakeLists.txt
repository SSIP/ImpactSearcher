cmake_minimum_required (VERSION 2.6)
project (ImpactSearcher CXX C)

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
add_library(imse SHARED image_helper.cpp main.cpp thread_2_center.cpp math_helpers.cpp thread_3_average.cpp image_bmp.cpp filesystem.cpp thread_4_presort.cpp thread_1_IO.cpp thread_5_check.cpp model_jupiter.cpp)
target_link_libraries (imse)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_compile_definitions(imse PUBLIC BUILDING_LIB _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
install(TARGETS imse DESTINATION lib)
install (FILES libimse.h DESTINATION include)
SET(CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mssse3")