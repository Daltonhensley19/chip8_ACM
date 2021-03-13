cmake_minimum_required(VERSION 3.17.5)

project(chip8)

set(SOURCE_FILES ../../src/main.cpp ../../src/chip8.cpp ../../src/utility.cpp)

# CXX Flags
set(set CMAKE_CXX_FLAGS " -O3 -L/usr/local/lib -lSDL2 -lfmt -std=c++20")

add_executable(chip8 ${SOURCE_FILES})

FIND_PACKAGE(SDL2 REQUIRED)
find_package(fmt)


INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${fmt_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(chip8 ${SDL2_LIBRARIES} fmt::fmt-header-only)









