#include "chip8.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <time.h>
/* Key:
nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
n or nibble - A 4-bit value, the lowest 4 bits of the instruction
x - A 4-bit value, the lower 4 bits of the high byte of the instruction
y - A 4-bit value, the upper 4 bits of the low byte of the instruction
kk or byte - An 8-bit value, the lowest 8 bits of the instruction
*/

std::uint8_t font_set[80]{
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
	pc = 0x200;
	sp = 0;
	opcode = 0;
	I = 0;

	// Clear Memory

	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}

	// Clear V Register, Stack, Keypad

	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
		V[i] = 0;
		keypad[i] = 0;
	}

	// Clear Display (Graphics buffer)

	for (int i = 0; i < 2048; ++i) {
		gfx[i] = 0;
	}

	// Clear Sound/Delay Timers

	delay_timer = 0;
	sound_timer = 0;

	// Generate a random seed based on current time
	srand(time(NULL));
}

void Chip8::execute_cycle() {
	opcode = memory[pc] << 8 | memory[pc + 1];

	switch (opcode & 0xF000) {
	case 0x0000:

		switch (opcode & 0x000F) {
		// Clear display
		case 0x0000:
			for (int i = 0; i < 2048; ++i) {
				gfx[i] = 0;
			}
			drawFlag = true;
			pc += 2;
			break;
		// Return from subroutine
		case 0x000E:
			--sp;
			pc = stack[sp];
			pc += 2;
			break;
		default:
			std::cout << "[ERROR]: Invalid opcode. "
				     "Segment 0000. "
				     "Draw-screen error."
				  << "\n";
			exit(3);

		} break;
	// Jump to location nnn
	case 0x1000:
		pc = opcode & 0x0FFF;
		break;
	// Call subroutine at nnn
	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	// Skip next instruction if Vx == kk
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;
	// Skip next instruction if Vx != kk
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;
	// Skip next instruction if Vx == Vy
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		} else {
			pc += 2;
			break;
		}
	// Set Vx = kk
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;
	// Set Vx += kk
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;
	case 0x8000:
		switch (opcode & 0x000F) {
		// Set Vx = Vy
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		// Set Vx |= Vy
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		// Set Vx &= Vy
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		// Set Vx ^= Vy
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		// Set Vx = Vx + Vy, set VF = carry if Vy > (0xF
		// - Vx)
		case 0x0004:
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			if (V[(opcode & 0x00F0) >> 4] >
			    (0x00FF - V[opcode & 0x0F00 >> 8])) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			pc += 2;
			break;
		// Set Vx = Vx - Vy, set VF = NOT borrow
		case 0x0005:
			if (V[(opcode & 0x0F00) >> 8] >
			    V[(opcode & 0x00F0) >> 4]) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		// Set Vx = Vx SHR 1
		case 0x0006:
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x0001;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;
		// Set Vx = Vy - Vx, set VF = NOT borrow.
		case 0x0007:
			if (V[(opcode & 0x00F0) >> 4] >
			    V[(opcode & 0x0F00) >> 8]) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] -
						    V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		// Set Vx = Vx SHL 1
		case 0x000E:
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;
		default:
			std::cout << "[ERROR]: Invalid opcode, "
				     "Segment 8000\n";
		}
		break;
	// Skip next instruction if Vx != Vy
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		} else {
			pc += 2;
		}
		break;
	// Set I = nnn
	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		break;
	case 0xC000:
		V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
		pc += 2;
		break;
	case 0xD000: {
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++) {
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++) {
				if ((pixel & (0x80 >> xline)) != 0) {
					if (gfx[(x + xline +
						 ((y + yline) * 64))] == 1) {
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^=
					    1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	} break;

	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E:
			if (keypad[V[(opcode & 0x0F00) >> 8]] != 0) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		case 0x00A1:
			if (keypad[V[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			} else {
				pc += 2;
			}
			break;
		default:
			std::cout << "[ERROR]: Unknown opcode. "
				     "Segment: 0xE000\n";
		}
		break;
	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;
		case 0x000A: {
			bool key_pushed = false;

			for (int i = 0; i < 16; ++i) {
				if (keypad[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					key_pushed = true;
				}
			}

			if (!key_pushed) {
				return;
			}
			pc += 2;
		} break;
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x001E:
			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {
				V[0xF] = 1;
			} else {
				V[0xF] = 0;
			}
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5; // 4x5 Sprite
			pc += 2;
			break;
		case 0x0033:
			memory[I] =
				(std::uint8_t)(V[(opcode & 0x0F00) >> 8] / 100);
			memory[I + 1] =
			   (std::uint8_t)(V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] =
			   (std::uint8_t) (V[(opcode & 0x0F00) >> 8] % 10);
			pc += 2;
			break;
		case 0x0055:
			for (int i = 0; i < ((opcode & 0x0F00) >> 8);
			     ++i) {
				memory[I + i] = V[i];
			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		case 0x0065:
			for (int i = 0; i < V[(opcode & 0x0F00) >> 8] + 1;
			     ++i) {
				V[i] = memory[I + i];
			}
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;
		default:
			std::cout << "[ERROR]: Invalid instruction. "
				     "Segment: F000\n";
		}
		break;
	default:
		std::cout << "[ERROR]: Invalid instruction. Invalid "
			     "segment.\n";
	}

	if (sound_timer > 0) {
		--sound_timer;
	}
	// Chip-8 requires the delay timer to decrement at a rate of 60Hz
	if (delay_timer > 0) {
		--delay_timer;
	}
}
// Initialise and load ROM into memory
bool Chip8::load_rom(const char *rom_path) {
	// Initialise
	init();
	std::cout << "Loading currently selected ROM: " << rom_path << "\n";

	// Open ROM file
	FILE *rom = fopen(rom_path, "rb");
	if (rom == NULL) {
		std::cerr << "Failed to open ROM" << std::endl;
		return false;
	}

	// Get file size
	fseek(rom, 0, SEEK_END);
	long rom_size = ftell(rom);
	rewind(rom);

	// Allocate memory to store rom
	char *rom_buffer = (char *)malloc(sizeof(char) * rom_size);
	if (rom_buffer == NULL) {
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
			memory[i + 512] =
			    (uint8_t)rom_buffer[i]; // Load into memory starting
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
