#include <cstdint>

struct Chip8 {

  // Chip-8 Specs

  std::uint16_t sp;        // Stack Pointer
  std::uint16_t stack[16]; // Stack

  std::uint8_t memory[4096]; // System Memory

  std::uint8_t delay_timer; // Delay Timer
  std::uint8_t sound_timer; // Sound Timer

  std::uint16_t pc;     // Program Counter
  std::uint8_t V[16];   // V Registers (V0-VF)
  std::uint16_t I;      // Index Register
  std::uint16_t opcode; // Current Opcode

  std::uint8_t keypad[16];   // Keypad
  std::uint8_t gfx[64 * 32]; // Graphics Buffer
  bool drawFlag;

  // Chip-8 Functions
  void init();
  void execute_cycle();
  bool load_rom(const char *rom_path);
};
