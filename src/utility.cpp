#include <iostream>

#include "../include/chip8.h"

// Initialise and load ROM into memory
bool Chip8::load_rom(const char *rom_path) {
  // Initialise
  init();
  std::cout << "Loading currently selected ROM: " << rom_path << "\n";

  // Open ROM file
  FILE *rom = fopen(rom_path, "rb");
  if (rom == nullptr) {
    std::cerr << "Failed to open ROM" << std::endl;
    return false;
  }

  // Get file size
  fseek(rom, 0, SEEK_END);
  long rom_size = ftell(rom);
  rewind(rom);

  // Allocate memory to store rom
  char *rom_buffer = (char *)malloc(sizeof(char) * rom_size);
  if (rom_buffer == nullptr) {
    std::cerr << "Failed to allocate memory for ROM" << std::endl;
    return false;
  }

  // Copy ROM into buffer
  size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
  if (result != rom_size) {
    std::cerr << "Failed to read ROM" << std::endl;
    return false;
  }

  // Copy buffer to memory
  if ((4096 - 512) > rom_size) {
    for (int i = 0; i < rom_size; ++i) {
      memory[i + 512] = (uint8_t)rom_buffer[i]; // Load into memory starting
                                                // at 0x200 (=512)
    }
  } else {
    std::cerr << "ROM too large to fit in memory" << std::endl;
    return false;
  }

  // Clean up
  fclose(rom);
  free(rom_buffer);

  return true;
}
