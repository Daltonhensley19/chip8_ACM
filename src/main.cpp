#include <iostream>
#include "SDL2/SDL.h"
#include "chip8.h"

#define WINDOW_HEIGHT 512
#define WINDOW_WIDTH 1024

int main(int argc, char** argv) {
	

	if (argc == 1) {
		std::cout << "Usage: ./chip8 [ROM]" << "\n";
		return 1;
	}
	// Initialize Chip-8 object
	Chip8 chip8;		
	

	SDL_Window* window;
	SDL_Renderer* renderer;


	return 0;
}
