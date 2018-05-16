#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include "arm.h"
#include "arm_mem.h"

#include "io.h"
#include "sdl.h"
#include "video.h"

#define JOY_A 0
#define JOY_B 1
#define JOY_X 2
#define JOY_Y 3
#define JOY_LSTICK 4
#define JOY_RSTICK 5
#define JOY_L 6
#define JOY_R 7
#define JOY_ZL 8
#define JOY_ZR 9
#define JOY_PLUS 10
#define JOY_MINUS 11
#define JOY_DLEFT 12
#define JOY_DUP 13
#define JOY_DRIGHT 14
#define JOY_DDOWN 15
#define JOY_LSTICK_LEFT 16
#define JOY_LSTICK_UP 17
#define JOY_LSTICK_RIGHT 18
#define JOY_LSTICK_DOWN 19
#define JOY_RSTICK_LEFT 20
#define JOY_RSTICK_UP 21
#define JOY_RSTICK_RIGHT 22
#define JOY_RSTICK_DOWN 23

const int64_t max_rom_sz = 32 * 1024 * 1024;

static uint32_t to_pow2(uint32_t val) {
    val--;

    val |= (val >>  1);
    val |= (val >>  2);
    val |= (val >>  4);
    val |= (val >>  8);
    val |= (val >> 16);

    return val + 1;
}

int main(int argc, char* argv[]) {
    consoleDebugInit(debugDevice_SVC);
    stdout = stderr;
    printf("gdkGBA - Gameboy Advance emulator made by gdkchan\n");
    printf("This is FREE software released into the PUBLIC DOMAIN\n\n");

    arm_init();


    FILE *image;

    image = fopen("/gba_bios.bin", "rb");

    if (image == NULL) {
        printf("Error: GBA BIOS not found!\n");
        printf("Place it on this directory with the name \"gba_bios.bin\".\n");

        return 0;
    }

    fread(bios, 16384, 1, image);

    fclose(image);

    image = fopen("/switch/roms/rom.gba", "rb");

    if (image == NULL) {
        printf("Error: ROM file couldn't be opened.\n");
        printf("Make sure that the file exists and the name is correct.\n");

        return 0;
    }

    fseek(image, 0, SEEK_END);

    cart_rom_size = ftell(image);
    cart_rom_mask = to_pow2(cart_rom_size) - 1;

    if (cart_rom_size > max_rom_sz) cart_rom_size = max_rom_sz;

    fseek(image, 0, SEEK_SET);
    fread(rom, cart_rom_size, 1, image);

    fclose(image);

    sdl_init();
    arm_reset();

    bool run = true;

    while (run) {
        run_frame();

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_JOYBUTTONDOWN:
                switch (event.jbutton.button) {
                    case JOY_DUP:           key_input.w &= ~BTN_U;   break;
                    case JOY_LSTICK_UP:     key_input.w &= ~BTN_U;   break;
                    case JOY_DDOWN:         key_input.w &= ~BTN_D;   break;
                    case JOY_LSTICK_DOWN:   key_input.w &= ~BTN_D;   break;
                    case JOY_DLEFT:         key_input.w &= ~BTN_L;   break;
                    case JOY_LSTICK_LEFT:   key_input.w &= ~BTN_L;   break;
                    case JOY_DRIGHT:        key_input.w &= ~BTN_R;   break;
                    case JOY_LSTICK_RIGHT:  key_input.w &= ~BTN_R;   break;
                    case JOY_A:             key_input.w &= ~BTN_A;   break;
                    case JOY_B:             key_input.w &= ~BTN_B;   break;
                    case JOY_L:             key_input.w &= ~BTN_LT;  break;
                    case JOY_R:             key_input.w &= ~BTN_RT;  break;
                    case JOY_MINUS:         key_input.w &= ~BTN_SEL; break;
                    case JOY_PLUS:          key_input.w &= ~BTN_STA; break;
                    default:  break;
                }
                break;

                case SDL_JOYBUTTONUP:
                switch (event.jbutton.button) {
                    case JOY_DUP:           key_input.w |= BTN_U;   break;
                    case JOY_LSTICK_UP:     key_input.w |= BTN_U;   break;
                    case JOY_DDOWN:         key_input.w |= BTN_D;   break;
                    case JOY_LSTICK_DOWN:   key_input.w |= BTN_D;   break;
                    case JOY_DLEFT:         key_input.w |= BTN_L;   break;
                    case JOY_LSTICK_LEFT:   key_input.w |= BTN_L;   break;
                    case JOY_DRIGHT:        key_input.w |= BTN_R;   break;
                    case JOY_LSTICK_RIGHT:  key_input.w |= BTN_R;   break;
                    case JOY_A:             key_input.w |= BTN_A;   break;
                    case JOY_B:             key_input.w |= BTN_B;   break;
                    case JOY_L:             key_input.w |= BTN_LT;  break;
                    case JOY_R:             key_input.w |= BTN_RT;  break;
                    case JOY_MINUS:         key_input.w |= BTN_SEL; break;
                    case JOY_PLUS:          key_input.w |= BTN_STA; break;
                    default:  break;
                }
                break;

                case SDL_QUIT: run = false; break;
            }
        }
    }

    sdl_uninit();
    arm_uninit();

    return 0;
}