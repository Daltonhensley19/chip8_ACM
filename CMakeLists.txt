cmake_minimum_required(VERSION 3.19.2)

project(chip8)

set(SOURCE_FILES src/main.cpp src/chip8.cpp)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8 ${SOURCE_FILES})
target_link_libraries(chip8 ${SDL2_LIBRARIES})
