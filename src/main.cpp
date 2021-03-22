#include "fmt/core.h"
#include "../lib/indicators/single_include/indicators/indicators.hpp"
#include "SDL2/SDL.h"
#include <chrono>
#include <thread>

#include "../include/chip8.h"

// Keypad keymap
static u8 keymap[16] = {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};

int main(int argc, char **argv) {

    using namespace indicators;
    show_console_cursor(false);

    ProgressBar bar{option::BarWidth{50},
                    option::Start{"["},
                    option::Fill{"■"},
                    option::Lead{"■"},
                    option::Remainder{"-"},
                    option::End{" ]"},
                    option::PostfixText{"Initializing Chip-8 1/2\n"},
                    option::ForegroundColor{Color::magenta},
                    option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};
    // Update bar state
    bar.set_progress(10); // 10% done

    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    bar.set_option(option::PostfixText{"Loading ROM 2/2 \n"});

    bar.set_progress(100); // 100% done


    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    show_console_cursor(true);
/* *****************************************************************************************/

// Display terminal usage
    if (argc != 2) {
        fmt::print("Usage: chip8 <ROM file> \n");
        return 1;
    }

    Chip8 chip8 = Chip8(); // Initialize Chip8

    const int w = 1024; // Window width
    const int h = 512;  // Window height

    // First, we will create a window
    SDL_Window *window;

    // Check to see if SDL can initialize
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fmt::format("SDL could not initialize! SDL_Error: {}\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        fmt::format("Window could not be created! SDL_Error: {}\n", SDL_GetError());
        exit(2);
    }

    // We then create the renderer for the window
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, w, h);

    // Create texture that stores frame buffer
    SDL_Texture *sdlTexture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    // Pixel buffer
    u32 pixels[2048];


    // Attempt to load ROM
    if (!chip8.load_rom(argv[1]))
        return 2;

    // Emulation loop
    while (true) {
        chip8.execute_cycle();

        // Process SDL events, such as the keyboard
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                exit(EXIT_SUCCESS);

            // Process keydown events
            if (e.type == SDL_KEYDOWN) {
                // Handle escape key to terminate program
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(EXIT_SUCCESS);

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.keypad[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.keypad[i] = 0;
                    }
                }
            }
        }

        // If draw occurred, redraw SDL screen
        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            // We will then store pixels in the temporary buffer
            for (int i = 0; i < 2048; ++i) {
                u8 pixel = chip8.gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // Update SDL texture with new batch of pixels
            SDL_UpdateTexture(sdlTexture, nullptr, pixels, 64 * sizeof(Uint32));
            // Clear the renderer
            SDL_RenderClear(renderer);
            // Copy updated SDL_Texture to the renderer
            SDL_RenderCopy(renderer, sdlTexture, nullptr, nullptr);
            // Update renderer with copied SDL_Texture
            SDL_RenderPresent(renderer);
        }

        // Slow down the running thread to pace the emulation
        std::this_thread::sleep_for(std::chrono::microseconds(1300));
    }
}
