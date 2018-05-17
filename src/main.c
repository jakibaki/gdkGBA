#include <stdio.h>
#include <stdlib.h>
#include "arm.h"
#include "arm_mem.h"
#include "bios.h"

#include "io.h"
#include "sdl.h"
#include "video.h"

#include <dirent.h>
#include <switch.h>

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

int getInd(char* curFile, int curIndex) {
    DIR* dir;
    struct dirent* ent;


    if(curIndex < 0)
        curIndex = 0;
    
    dir = opendir("/switch/roms/gba");//Open current-working-directory.
    if(dir==NULL)
    {
        sprintf(curFile, "Failed to open dir!");
        return curIndex;
    }
    else
    {
        int i;
        for(i = 0; i <= curIndex; i++) {
            ent = readdir(dir);
        }
        if(ent)
            sprintf(curFile ,"/switch/roms/gba/%s", ent->d_name);
        else
            curIndex--;
        closedir(dir);
    }
    return curIndex;
}

void getFile(char* curFile)
{
 
    gfxInitDefault();

    //Initialize console. Using NULL as the second argument tells the console library to use the internal console structure as current one.
    consoleInit(NULL);

    //Move the cursor to row 16 and column 20 and then prints "Hello World!"
    //To move the cursor you have to print "\x1b[r;cH", where r and c are respectively
    //the row and column where you want your cursor to move
    printf("\x1b[16;20HSelect a file using the up and down keys.");
    printf("\x1b[17;20HPress start to run the rom.");

    sprintf(curFile, "Couldn't find any files in that folder!");
    int curIndex = 0;

    curIndex = getInd(curFile, curIndex);
    printf("\x1b[18;20H%s", curFile);
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_DOWN || kDown & KEY_DDOWN) {
            consoleClear();
            printf("\x1b[16;20HSelect a file using the up and down keys.");
            printf("\x1b[17;20HPress start to run the rom.");
            curIndex++;
            curIndex = getInd(curFile, curIndex);
            printf("\x1b[18;20H%s", curFile);
        }

        if (kDown & KEY_UP || kDown & KEY_DUP) {
            consoleClear();
            printf("\x1b[16;20HSelect a file using the up and down keys.");
            printf("\x1b[17;20HPress start to run the rom.");
            curIndex--;
            curIndex = getInd(curFile, curIndex);
            printf("\x1b[18;20H%s", curFile);
        }


        if (kDown & KEY_PLUS || kDown & KEY_A) {
            
            break;
        }  
        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
    
    
    consoleClear();
    gfxExit();
}



int main(int argc, char* argv[]) {
    
    char filename[100];
    char savegamepath[110];
    getFile(filename);
    sprintf(savegamepath, "%s.savegame", filename);

    arm_init();
    sdl_init();



    memcpy(bios, gba_bios, 16384);


    FILE* image = fopen(filename, "rb");

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

    arm_reset();

    bool run = true;
    bool startDown = false;

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
                    case JOY_ZL:
                    case JOY_L:             
                        if(startDown)
                            arm_load(savegamepath);
                        key_input.w &= ~BTN_LT;  
                        break;
                    case JOY_ZR:
                    case JOY_R:           
                        if(startDown)
                            arm_save(savegamepath);
                        else
                            key_input.w &= ~BTN_RT;  
                        break;
                    case JOY_MINUS:         key_input.w &= ~BTN_SEL; break;
                    case JOY_PLUS:          key_input.w &= ~BTN_STA; startDown = true; break;
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
                    case JOY_ZL:
                    case JOY_L:             key_input.w |= BTN_LT;  break;
                    case JOY_ZR:
                    case JOY_R:             key_input.w |= BTN_RT;  break;
                    case JOY_MINUS:         key_input.w |= BTN_SEL; break;
                    case JOY_PLUS:          key_input.w |= BTN_STA; startDown = false; break;
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