#include <chrono>

#include "../include/chip8.h"

#include "../lib/fmt/include/fmt/core.h"

#define Vx V[(opcode & 0x0F00) >> 8]
#define Vy V[(opcode & 0x00F0) >> 4]


/* Key:
nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
n or nibble - A 4-bit value, the lowest 4 bits of the instruction
x - A 4-bit value, the lower 4 bits of the high byte of the instruction
y - A 4-bit value, the upper 4 bits of the low byte of the instruction
kk or byte - An 8-bit value, the lowest 8 bits of the instruction
*/
enum opcodes {
    Opcode00E0 = 0x0000,
    Opcode00EE = 0x000E,
    Opcode1nnn = 0x1000,
    Opcode2nnn = 0x2000,
    Opcode3xkk = 0x3000,
    Opcode4xkk = 0x4000,
    Opcode5xy0 = 0x5000,
    Opcode6xkk = 0x6000,
    Opcode7xkk = 0x7000,
    Opcode8xy0 = 0x0000,
    Opcode8xy1 = 0x0001,
    Opcode8xy2 = 0x0002,
    Opcode8xy3 = 0x0003,
    Opcode8xy4 = 0x0004,
    Opcode8xy5 = 0x0005,
    Opcode8xy6 = 0x0006,
    Opcode8xy7 = 0x0007,
    Opcode8xyE = 0x000E,
    Opcode9xy0 = 0x9000,
    OpcodeAnnn = 0xA000,
    OpcodeBnnn = 0xB000,
    OpcodeCxkk = 0xC000,
    OpcodeDxyn = 0xD000,
    OpcodeEx9E = 0x009E,
    OpcodeExA1 = 0x00A1,
    OpcodeFx07 = 0x0007,
    OpcodeFx0A = 0x000A,
    OpcodeFx15 = 0x0015,
    OpcodeFx18 = 0x0018,
    OpcodeFx1E = 0x001E,
    OpcodeFx29 = 0x0029,
    OpcodeFx33 = 0x0033,
    OpcodeFx55 = 0x0055,
    OpcodeFx65 = 0x0065
};

const int FONT_SIZE = 80;
static u8 font_set[FONT_SIZE]{
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::init() {

    // Clear Memory

    for (unsigned char &i : memory) {
        i = 0;
    }

    // Clear V Register, Stack, Keypad

    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        V[i] = 0;
        keypad[i] = 0;
    }

    // Clear Display (Graphics buffer)

    for (unsigned char &i : gfx) {
        i = 0;
    }
    // Load Chip-8 font into memory
    for (int i = 0; i < FONT_SIZE; i++) {
        memory[i] = font_set[i];
    }

    // Clear Sound/Delay Timers

    delay_timer = 0;
    sound_timer = 0;

    // Generate a random seed based on current time
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
}

// In order to emulate the Chip-8 on a cycle-level, we have to use the
// fetch-decode-execute process.
void Chip8::execute_cycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x0000:

            switch (opcode & 0x000F) {
                // Clear display
                case Opcode00E0:
                    fmt::print("Current Instruction: CLS\n");
                    for (unsigned char &i : gfx) {
                        i = 0;
                    }
                    drawFlag = true;
                    pc += 2;
                    break;
                    // Return from subroutine
                case Opcode00EE:
                    fmt::print("Current Instruction: RET\n");
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    fmt::format("[ERROR]: Invalid opcode. Segment 0000. Draw-screen error. "
                                "Opcode: {}\n",
                                opcode);
                    exit(3);
            }
            break;
            // Jump to location nnn
        case Opcode1nnn:
            fmt::print("Current Instruction: JP\n");
            pc = opcode & 0x0FFF;
            break;
            // Call subroutine at nnn
        case Opcode2nnn:
            fmt::print("Current Instruction: CALL\n");
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;
            // Skip next instruction if Vx == kk
        case Opcode3xkk:
            fmt::print("Current Instruction: SE Vx, byte\n");
            if (Vx == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
            // Skip next instruction if Vx != kk
        case Opcode4xkk:
            fmt::print("Current Instruction: SNE Vx, byte\n");
            if (Vx != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
            // Skip next instruction if Vx == Vy
        case Opcode5xy0:
            fmt::print("Current Instruction: SE Vx, Vy\n");
            if (Vx == Vy) {
                pc += 4;
            } else {
                pc += 2;
                break;
            }
            // Set Vx = kk
        case Opcode6xkk:
            fmt::print("Current Instruction: LD Vx, byte\n");
            Vx = (opcode & 0x00FF);
            pc += 2;
            break;
            // Set Vx += kk
        case Opcode7xkk:
            fmt::print("Current Instruction: ADD Vx, byte\n");
            Vx += (opcode & 0x00FF);
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                // Set Vx = Vy
                case Opcode8xy0:
                    fmt::print("Current Instruction: LD Vx, Vy\n");
                    Vx = Vy;
                    pc += 2;
                    break;
                    // Set Vx |= Vy
                case Opcode8xy1:
                    fmt::print("Current Instruction: OR Vx, Vy\n");
                    Vx |= Vy;
                    pc += 2;
                    break;
                    // Set Vx &= Vy
                case Opcode8xy2:
                    fmt::print("Current Instruction: AND Vx, Vy\n");
                    Vx &= Vy;
                    pc += 2;
                    break;
                    // Set Vx ^= Vy
                case Opcode8xy3:
                    fmt::print("Current Instruction: XOR Vx, Vy\n");
                    Vx ^= Vy;
                    pc += 2;
                    break;
                    // Set Vx = Vx + Vy, set VF = carry if Vy > (0xF
                    // - Vx)
                case Opcode8xy4:
                    fmt::print("Current Instruction: ADD Vx, Vy\n");
                    Vx += Vy;
                    if (Vy > (0x00FF - Vx)) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    pc += 2;
                    break;
                    // Set Vx = Vx - Vy, set VF = NOT borrow
                case Opcode8xy5:
                    fmt::print("Current Instruction: SUB Vx, Vy\n");
                    if (Vx > Vy) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    Vx -= Vy;
                    pc += 2;
                    break;
                    // Set Vx = Vx SHR 1
                case Opcode8xy6:
                    fmt::print("Current Instruction: SHR Vx\n");
                    V[0xF] = Vx & 0x0001;
                    Vx >>= 1;
                    pc += 2;
                    break;
                    // Set Vx = Vy - Vx, set VF = NOT borrow.
                case Opcode8xy7:
                    fmt::print("Current Instruction: SUBN Vx, Vy\n");
                    if (Vy > Vx) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    Vx = Vy - Vx;
                    pc += 2;
                    break;
                    // Set Vx = Vx SHL 1
                case Opcode8xyE:
                    fmt::print("Current Instruction: SHL Vx\n");
                    V[0xF] = Vx >> 7;
                    Vx <<= 1;
                    pc += 2;
                    break;
                default:
                    fmt::format("[ERROR]: Invalid opcode Segment 8000. Opcode: {}\n", opcode);
            }
            break;
            // Skip next instruction if Vx != Vy
        case Opcode9xy0:
            fmt::print("Current Instruction: SNE Vx, Vy\n");
            if (Vx != Vy) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
            // Set I = nnn
        case OpcodeAnnn:
            fmt::print("Current Instruction: LD I, addr\n");
            I = opcode & 0x0FFF;
            pc += 2;
            break;
            // Jump to location nnn + V0
        case OpcodeBnnn:
            fmt::print("Current Instruction: JP V0, addr\n");
            pc = (opcode & 0x0FFF) + V[0];
            pc += 2;
            break;
            // Set Vx = random byte AND kk
        case OpcodeCxkk:
            fmt::print("Current Instruction: RND Vx, byte\n");
            Vx = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set
            // VF = collision
        case OpcodeDxyn: {
            fmt::print("Current Instruction: DRW Vx, Vy\n");
            unsigned short x = Vx;
            unsigned short y = Vy;
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                            V[0xF] = 1;
                        }
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        }
            break;

        case 0xE000:
            switch (opcode & 0x00FF) {
                // Skip next instruction if key with the value of Vx is pressed
                case OpcodeEx9E:
                    fmt::print("Current Instruction: SKP Vx\n");
                    if (keypad[Vx] != 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                    // Skip next instruction if key with the value of Vx is not
                    // pressed
                case OpcodeExA1:
                    fmt::print("Current Instruction: SKNP Vx\n");
                    if (keypad[Vx] == 0) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                default:
                    fmt::format("[ERROR]: Unknown opcode. Segment: 0xE000. Opcode: {}\n",
                                opcode);
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                // Set Vx = delay timer value
                case OpcodeFx07:
                    fmt::print("Current Instruction: LD Vx, DT\n");
                    Vx = delay_timer;
                    pc += 2;
                    break;
                    // Wait for a key press, store the value of the key in Vx
                case OpcodeFx0A: {
                    fmt::print("Current Instruction: LD Vx, K\n");
                    bool key_pushed = false;

                    for (int i = 0; i < 16; i++) {
                        if (keypad[i] != 0) {
                            Vx = i;
                            key_pushed = true;
                        }
                    }

                    if (!key_pushed) {
                        return;
                    }
                    pc += 2;
                }
                    break;
                    // Set delay timer = Vx
                case OpcodeFx15:
                    fmt::print("Current Instruction: LD DT, Vx\n");
                    delay_timer = Vx;
                    pc += 2;
                    break;
                    // Set sound timer = Vx
                case OpcodeFx18:
                    fmt::print("Current Instruction: LD ST, Vx\n");
                    sound_timer = Vx;
                    pc += 2;
                    break;
                    // Set I = I + Vx
                case OpcodeFx1E:
                    fmt::print("Current Instruction: ADD I, Vx\n");
                    if (I + Vx > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += Vx;
                    pc += 2;
                    break;
                    // Set I = location of sprite for digit Vx
                case OpcodeFx29:
                    fmt::print("Current Instruction: LD F, Vx\n");
                    I = Vx * 0x5; // 4x5 Sprite
                    pc += 2;
                    break;
                    // Store BCD representation of Vx in memory locations I, I+1,
                    // and I+2
                case OpcodeFx33:
                    fmt::print("Current Instruction: LD B, Vx\n");
                    memory[I] = Vx / 100;
                    memory[I + 1] = (Vx / 10) % 10;
                    memory[I + 2] = Vx % 10;
                    pc += 2;
                    break;
                    // Store registers V0 through Vx in memory starting at location
                    // I
                case OpcodeFx55:
                    fmt::print("Current Instruction: LD [I], Vx\n");
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = V[i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                    // Read registers V0 through Vx from memory starting at location
                    // I
                case OpcodeFx65:
                    fmt::print("Current Instruction: LD Vx, [I]\n");
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        V[i] = memory[I + i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                default:
                    fmt::format("[ERROR]: Invalid instruction. Segment: F000. Opcode: {}\n",
                                opcode);
            }
            break;
        default:
            fmt::format("[ERROR]: Unimplemented instruction. Invalid opcode: {}\n",
                        opcode);
    }

    if (sound_timer > 0) {
        --sound_timer;
    }
    // Chip-8 requires the delay timer to decrement at a rate of 60Hz
    if (delay_timer > 0) {
        --delay_timer;
    }
}
