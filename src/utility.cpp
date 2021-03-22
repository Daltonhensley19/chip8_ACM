#include <iostream>
#include <fstream>

#include "../include/chip8.h"
#include "fmt/core.h"

// Initialize and load ROM into memory
const u16 MEMORY_START = 512;

bool Chip8::load_rom(const char *rom_path) {
    // Initialize
    init();

    std::ifstream infile(rom_path, std::ios::binary | std::ios::ate); // Read ROM starting at end and in binary mode

    // Check to see if successful
    if (!infile.good()) {
        fmt::format("Error! Could not read file: {} ", rom_path);
        return false;
    } else {
        std::streampos size = infile.tellg(); // Get ROM size
        char *buffer = new char[size];

        infile.seekg(0, std::ios::beg); // Move read position to beginning of ROM
        infile.read(buffer, size);
        infile.close();

        for (long i = 0; i < size; i++) {
            memory[i + MEMORY_START] = (u8) buffer[i]; // Chip-8 memory starts at 512 bytes
        }

        delete[] buffer;
    }
    return true;

}
