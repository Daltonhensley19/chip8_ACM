#include <cstdint>

using u8 = std::uint8_t;
using u16 = std::uint16_t;

struct Chip8 {
    // Using member initializer list with the Chip8 constructor instead of
    // using the Chip8::init() function
    Chip8() : pc(0x200), sp(0), opcode(0), I(0) {}

    // Chip-8 Specs

    u16 sp;        // Stack Pointer, points to top of the stack
    u16 stack[16]; // Stack

    u8 memory[4096]; // System Memory, 4KB

    u8 delay_timer; // Delay Timer
    u8 sound_timer; // Sound Timer

    u16 pc;     // Program Counter
    u8 V[16];   // V Registers (V0-VF)
    u16 I;      // Index Register
    u16 opcode; // Current Opcode

    u8 keypad[16];   // Keypad
    u8 gfx[64 * 32]; // Graphics Buffer, total size = 2048 bytes (64*32)
    bool drawFlag;

    // Chip-8 Functions
    void init(); // Function to initialize

    void execute_cycle();

    bool load_rom(const char *rom_path);
};
