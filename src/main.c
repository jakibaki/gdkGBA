#include <stdio.h>
#include <stdlib.h>
#include "arm.h"
#include "arm_mem.h"
#include "bios.h"

#include "io.h"
#include "sdl.h"
#include "video.h"

#include <unistd.h>

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
bool debug_mode = 0;

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

void drawROMList(int curIndex) //Draws a list of files to the screen
{
    DIR* dir;
    struct dirent* ent;

    int i = 0;
    int start = curIndex - 15; //Defines when the function starts to draw the list
    if (start < 0) //Makes sure that it starts at 0, which is needed to make sure that start+20 is always drawn to the screen
    {
        start = 0;
    }
    
    dir = opendir("/switch/roms/gba");//Open current-working-directory.

    if (dir==NULL)
    {
        printf("\x1b[%d;4H-->%s", 6, "Failed to open dir!");
    }
    else
    {
        bool working = true;
        while (working) //While working it runs through the dir list
        {
            ent = readdir(dir);
            if (ent)
            {
                if (i >= start)
                {
                    if (i < (start+40))
                    {
                        if (i == curIndex)
                        {
                            printf("\x1b[%d;4H-->%s", 6 + (i - start), ent->d_name);
                        }
                        else
                        {
                            printf("\x1b[%d;4H%s", 6 + (i - start), ent->d_name);
                        }
                    }
                    else
                    {
                        closedir(dir);
                        working = false;
                    }
                }
            }
            else
            {
                closedir(dir);
                working = false;
            }
            i++;
        }
    }
}

void getFile(char* curFile)
{
 
    gfxInitDefault();

    //Initialize console. Using NULL as the second argument tells the console library to use the internal console structure as current one.
    consoleInit(NULL);

    //Move the cursor to row 16 and column 20 and then prints "Hello World!"
    //To move the cursor you have to print "\x1b[r;cH", where r and c are respectively
    //the row and column where you want your cursor to move
    int curIndex = 0;
    int heldTime = 0;
    int heldDelay = 8;

    curIndex = getInd(curFile, curIndex);

    drawROMList(curIndex);//Draws the ROMList at start

    while(appletMainLoop())
    {        
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
        
        printf("\x1b[3;4HSelect a file using the up and down keys.");
        printf("\x1b[4;4HPress start to run the rom.");
        
        if(debug_mode) {
            printf("\x1b[2;4HWelcome to debug mode!");
        }

        if (kDown & KEY_DOWN || kDown & KEY_DDOWN) {
            consoleClear();
            curIndex++;
            curIndex = getInd(curFile, curIndex);
            drawROMList(curIndex);
        }

        if (kDown & KEY_UP || kDown & KEY_DUP) {
            consoleClear();
            curIndex--;
            curIndex = getInd(curFile, curIndex);
            drawROMList(curIndex);
        }

        if (kHeld & KEY_DOWN || kHeld & KEY_DDOWN)
        {
            if (heldTime < heldDelay)
            {
                heldTime++;
            }
            else
            {
                consoleClear();
                curIndex++;
                curIndex = getInd(curFile, curIndex);
                drawROMList(curIndex);
            }
        }
        else if (kHeld & KEY_UP || kHeld & KEY_DUP)
        {
            if (heldTime < heldDelay)
            {
                heldTime++;
            }
            else
            {
                consoleClear();
                curIndex--;
                curIndex = getInd(curFile, curIndex);
                drawROMList(curIndex);
            }
        }
        else
        {
            heldTime = 0;
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
    
    for (int i=0; i<argc; i++) {
        // nxlink -a 192.168... gdkGBA.nro -s debug
        if(!strcmp(argv[i],"debug"))
            debug_mode = true;
    }

    char filename[100];
    getFile(filename);
    
    
    sprintf(savegamepath, "%s.sav", filename);

    char savestatepath[110];
    sprintf(savestatepath, "%s.savegame", filename);

    socketInitializeDefault();
    
    if(!debug_mode) {
        FILE* fakefile = fopen("/gdkGBA.tmp", "wb");
        stdout = fakefile;
        stderr = fakefile;
        fclose(fakefile);
        unlink("/gdkGBA.tmp");
        // The emulator crashes if there's something written into stdout or stderr for some reason D:.
        // Please let me know if you know of a better way to fix this
    } else {
        nxlinkStdio();
        printf("Connected to nxlink :)\n");
    }

    arm_init();
    sdl_init();
    

    FILE* biosfile = fopen("/switch/gba_bios.bin", "rb");

    if(biosfile != NULL) {
        fread(bios, 16384, 1, biosfile);
    } else {
        memcpy(bios, gba_bios, 16384);
    }

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
    arm_load(savegamepath);

    bool run = true;
    bool startDown = false;
    lastsaveused = -1;

    while (run) {
        run_frame();

        if(lastsaveused-- == 0) {
            arm_save(savegamepath);
        }

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
                            arm_loadstate(savestatepath);
                        key_input.w &= ~BTN_LT;  
                        break;
                    case JOY_ZR:
                    case JOY_R:           
                        if(startDown)
                            arm_savestate(savestatepath);
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