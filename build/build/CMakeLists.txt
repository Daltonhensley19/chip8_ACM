cmake_minimum_required(VERSION 3.17.5)

project(chip8)

set(SOURCE_FILES ../../src/main.cpp ../../src/chip8.cpp)

# CXX Flags
set(set CMAKE_CXX_FLAGS "-std=c++20 -O3") 

add_executable(chip8 ${SOURCE_FILES})

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8 ${SDL2_LIBRARIES})





