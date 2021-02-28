/*
  PokeMini - Pok�mon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "PokeMini.h"
#include "Hardware.h"
#include "Joystick.h"

#include "UI.h"
#include "Video_x3.c"
#include "PokeMini_BG3.c"

#include <mmenu.h>

const char *AppName = "PokeMini " PokeMini_Version " Trimui";

static SDL_Rect rct;

// Sound buffer size
#define SOUNDBUFFER	1024
#define PMSOUNDBUFF	(SOUNDBUFFER*2)

static SDL_Surface* rl_screen;
uint32_t x, y;
uint32_t *s, *d;
// --------

/* alekmaul's scaler taken from mame4all */
static void bitmap_scale(uint32_t startx, uint32_t starty, uint32_t viswidth, uint32_t visheight, uint32_t newwidth, uint32_t newheight,uint32_t pitchsrc,uint32_t pitchdest, uint16_t* __restrict__ src, uint16_t* __restrict__ dst)
{
    uint32_t W,H,ix,iy,x,y;
    x=startx<<16;
    y=starty<<16;
    W=newwidth;
    H=newheight;
    ix=(viswidth<<16)/W;
    iy=(visheight<<16)/H;

    do 
    {
        uint16_t* __restrict__ buffer_mem=&src[(y>>16)*pitchsrc];
        W=newwidth; x=startx<<16;
        do 
        {
            *dst++=buffer_mem[x>>16];
            x+=ix;
        } while (--W);
        dst+=pitchdest;
        y+=iy;
    } while (--H);
}

// Joystick names and mapping (NEW IN 0.5.0)
char *Joy_KeysNames[] = {
	"Off",		// -1
	"Select",	// 0
	"Start",	// 1
	"Up",		// 2
	"Down",		// 3
	"Left",		// 4
	"Right",	// 5
	"A",		// 6
	"B",		// 7
	"X",		// 8
	"Y",		// 9
	"L",		// 10
	"R",		// 11
	"Menu"		// 12
};
int Joy_KeysMapping[] = {
	12,		// Menu
	6,		// A
	7,		// B
	11,		// C
	2,		// Up
	3,		// Down
	4,		// Left
	5,		// Right
	-1,		// Power
	10		// Shake
};

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
void ScalingEnterMenu(void);

TUIMenu_Item UIItems_Platform[] = {
	//PLATFORMDEF_GOBACK,
	{ 0,  2, "Scaling settings", UIItems_PlatformC },
	{ 0,  3, "Button mapping", UIItems_PlatformC },
	PLATFORMDEF_SAVEOPTIONS,
	PLATFORMDEF_END(UIItems_PlatformC)
};

int UIItems_PlatformC(int index, int reason)
{
	UIMenu_ChangeItem(UIItems_Platform, 2, "Scaling: %s", CommandLine.scaling ? "Fullscreen" : "Integer");
	
	if (reason == UIMENU_OK) reason = UIMENU_RIGHT;
	if (reason == UIMENU_CANCEL) UIMenu_PrevMenu();
	if (reason == UIMENU_LEFT) {
		switch (index)
		{
			case 2:
				CommandLine.scaling = 0;
				UIMenu_ChangeItem(UIItems_Platform, 2, "Scaling: %s", CommandLine.scaling ? "Fullscreen" : "Integer");
			break;
		}
	}
	if (reason == UIMENU_RIGHT) {
		switch (index)
		{
			case 2:
				CommandLine.scaling = 1;
				UIMenu_ChangeItem(UIItems_Platform, 2, "Scaling: %s", CommandLine.scaling ? "Fullscreen" : "Integer");
			break;
			case 3:
				JoystickEnterMenu();
			break;
		}
	}
	return 1;
}


// For the emulator loop and video
int emurunning = 1;
SDL_Surface *screen;
int PixPitch, ScOffP;

#define TRIMUI_UP SDLK_UP
#define TRIMUI_DOWN SDLK_DOWN
#define TRIMUI_LEFT SDLK_LEFT
#define TRIMUI_RIGHT SDLK_RIGHT
#define TRIMUI_A SDLK_SPACE
#define TRIMUI_B SDLK_LCTRL
#define TRIMUI_X SDLK_LSHIFT
#define TRIMUI_Y SDLK_LALT
#define TRIMUI_L SDLK_TAB
#define TRIMUI_R SDLK_BACKSPACE
#define TRIMUI_START SDLK_RETURN
#define TRIMUI_SELECT SDLK_RCTRL
#define TRIMUI_MENU SDLK_ESCAPE

// Handle keyboard and quit events
void handleevents(SDL_Event *event)
{
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == TRIMUI_SELECT) {		// Dingoo Select
			JoystickButtonsEvent(0, 1);
		} else if (event->key.keysym.sym == TRIMUI_START) {	// Dingoo Start
			JoystickButtonsEvent(1, 1);
		} else if (event->key.keysym.sym == TRIMUI_UP) {		// Dingoo Up
			JoystickButtonsEvent(2, 1);
		} else if (event->key.keysym.sym == TRIMUI_DOWN) {	// Dingoo Down
			JoystickButtonsEvent(3, 1);
		} else if (event->key.keysym.sym == TRIMUI_LEFT) {	// Dingoo Left
			JoystickButtonsEvent(4, 1);
		} else if (event->key.keysym.sym == TRIMUI_RIGHT) {	// Dingoo Right
			JoystickButtonsEvent(5, 1);
		} else if (event->key.keysym.sym == TRIMUI_A) {	// Dingoo A
			JoystickButtonsEvent(6, 1);
		} else if (event->key.keysym.sym == TRIMUI_B) {	// Dingoo B
			JoystickButtonsEvent(7, 1);
		} else if (event->key.keysym.sym == TRIMUI_X) {	// Dingoo X
			JoystickButtonsEvent(8, 1);
		} else if (event->key.keysym.sym == TRIMUI_Y) {	// Dingoo Y
			JoystickButtonsEvent(9, 1);
		} else if (event->key.keysym.sym == TRIMUI_L) {		// Dingoo L1
			JoystickButtonsEvent(10, 1);
		} else if (event->key.keysym.sym == TRIMUI_R) {	// Dingoo R1
			JoystickButtonsEvent(11, 1);
		} else if (event->key.keysym.sym == TRIMUI_MENU) {	// Dingoo R
			JoystickButtonsEvent(12, 1);
		}
		break;
	case SDL_KEYUP:
		if (event->key.keysym.sym == TRIMUI_SELECT) {		// Dingoo Select
			JoystickButtonsEvent(0, 0);
		} else if (event->key.keysym.sym == TRIMUI_START) {	// Dingoo Start
			JoystickButtonsEvent(1, 0);
		} else if (event->key.keysym.sym == TRIMUI_UP) {	// Dingoo Up
			JoystickButtonsEvent(2, 0);
		} else if (event->key.keysym.sym == TRIMUI_DOWN) {	// Dingoo Down
			JoystickButtonsEvent(3, 0);
		} else if (event->key.keysym.sym == TRIMUI_LEFT) {	// Dingoo Left
			JoystickButtonsEvent(4, 0);
		} else if (event->key.keysym.sym == TRIMUI_RIGHT) {	// Dingoo Right
			JoystickButtonsEvent(5, 0);
		} else if (event->key.keysym.sym == TRIMUI_A) {	// Dingoo A
			JoystickButtonsEvent(6, 0);
		} else if (event->key.keysym.sym == TRIMUI_B) {	// Dingoo B
			JoystickButtonsEvent(7, 0);
		} else if (event->key.keysym.sym == TRIMUI_X) {	// Dingoo X
			JoystickButtonsEvent(8, 0);
		} else if (event->key.keysym.sym == TRIMUI_Y) {	// Dingoo Y
			JoystickButtonsEvent(9, 0);
		} else if (event->key.keysym.sym == TRIMUI_L) {		// Dingoo L1
			JoystickButtonsEvent(10, 0);
		} else if (event->key.keysym.sym == TRIMUI_R) {	// Dingoo R1
			JoystickButtonsEvent(11, 0);
		} else if (event->key.keysym.sym == TRIMUI_MENU) {	// Dingoo R
			JoystickButtonsEvent(12, 0);
		}
		break;
	case SDL_QUIT:
		emurunning = 0;
		break;
	};
}

// Used to fill the sound buffer
void emulatorsound(void *unused, Uint8 *stream, int len)
{
	MinxAudio_GetSamplesS16((int16_t *)stream, len>>1);
}

// Enable / Disable sound
void enablesound(int sound)
{
	MinxAudio_ChangeEngine(sound);
	if (AudioEnabled) SDL_PauseAudio(!sound);
}

static void Clear_Screen()
{
	uint32_t i;
	for(i=0;i<3;i++)
	{
		SDL_FillRect(screen, NULL, 0);
		SDL_FillRect(rl_screen, NULL, 0);
		SDL_Flip(rl_screen);
	}
}

// Menu loop
void menuloop()
{
	SDL_Rect mnu;
	mnu.x = 0;
	mnu.y = 0;
	mnu.w = 320;
	mnu.h = 240;
	SDL_Event event;
	
	// Stop sound
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	enablesound(0);

	// Update EEPROM
	PokeMini_SaveFromCommandLines(0);

	// Menu's loop
	Clear_Screen();
	
	while (emurunning && (UI_Status == UI_STATUS_MENU)) {
		// Slowdown to approx. 60fps
		SDL_Delay(16);

		// Process UI
		UIMenu_Process();

		// Screen rendering
		//SDL_FillRect(screen, NULL, 0);
		if (SDL_LockSurface(screen) == 0) {
			// Render the menu or the game screen
			UIMenu_Display_16((uint16_t *)screen->pixels + ScOffP, PixPitch);
			// Unlock surface
			SDL_UnlockSurface(screen);
			SDL_SoftStretch(screen, &mnu, rl_screen, NULL);
			SDL_Flip(rl_screen);
		}

		// Handle events
		while (SDL_PollEvent(&event)) handleevents(&event);
	}
	
	Clear_Screen();

	// Apply configs
	PokeMini_ApplyChanges();
	if (UI_Status == UI_STATUS_EXIT) emurunning = 0;
	else enablesound(CommandLine.sound);
	SDL_EnableKeyRepeat(0, 0);
}

char home_path[256];
char conf_path[512];
char save_path[256];
static void* mmenu = NULL;

// Main function
int main(int argc, char **argv)
{
	rct.x = 16;
	rct.y = 24;
	rct.w = 288;
	rct.h = 192;
	SDL_Joystick *joy;
	SDL_Event event;
	SDL_Rect pos;
	pos.x = 16;
	pos.y = 24;

	// Process arguments
	printf("%s\n\n", AppName);
	PokeMini_InitDirs(argv[0], NULL);
	
	snprintf(home_path, sizeof(home_path), "%s/.pokemini", getenv("HOME"));
	snprintf(conf_path, sizeof(conf_path), "%s/pokemini.cfg", home_path);
	if (access( home_path, F_OK ) == -1) {
		mkdir(home_path, 0755);
	}
	
	CommandLineInit();
	
	JoystickSetup("Trimui", 0, 30000, Joy_KeysNames, 13, Joy_KeysMapping);
	
	CommandLineConfFile(conf_path, NULL, NULL);
	if (!CommandLineArgs(argc, argv, NULL)) {
		PrintHelpUsage(stdout);
		return 1;
	}

	strcpy(save_path, home_path);
	char* tmp = save_path;
	tmp += strlen(tmp);
	strcpy(tmp, strrchr(CommandLine.min_file, '/'));
	tmp += strlen(tmp);
	strcpy(tmp, ".st%i");
	
	mmenu = dlopen("libmmenu.so", RTLD_NOW);
    if (!mmenu) {
    	printf("Cannot open mmenu: %s\n", dlerror());
		fflush(stdout);
    }

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	joy = SDL_JoystickOpen(0);
	atexit(SDL_Quit); // Clean up on exit

	// Set video spec and check if is supported
	if (!PokeMini_SetVideo((TPokeMini_VideoSpec *)&PokeMini_Video3x3, 16, CommandLine.lcdfilter, CommandLine.lcdmode)) {
		fprintf(stderr, "Couldn't set video spec\n");
		exit(1);
	}
	UIMenu_SetDisplay(288, 192, PokeMini_BGR16, (uint8_t *)PokeMini_BG3, (uint16_t *)PokeMini_BG3_PalBGR16, (uint32_t *)PokeMini_BG3_PalBGR32);

	// Initialize the display
	
	rl_screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
	if (rl_screen == NULL) {
		fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
		exit(1);
	}
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16, 0,0,0,0);

	PixPitch = screen->pitch / 2;
	ScOffP = (24 * PixPitch) + 16;

	// Initialize the sound
	SDL_AudioSpec audfmt, outfmt;
	audfmt.freq = 44100;
	audfmt.format = AUDIO_S16SYS;
	audfmt.channels = 1;
	audfmt.samples = SOUNDBUFFER;
	audfmt.callback = emulatorsound;
	audfmt.userdata = NULL;

	// Open the audio device
	if (SDL_OpenAudio(&audfmt, &outfmt) < 0) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		fprintf(stderr, "Audio will be disabled\n");
		AudioEnabled = 0;
	} else {
		AudioEnabled = 1;
	}

	// Disable key repeat and hide cursor
	SDL_EnableKeyRepeat(0, 0);
	SDL_ShowCursor(SDL_DISABLE);

	// Initialize the emulator
	printf("Starting emulator...\n");
	if (!PokeMini_Create(0, PMSOUNDBUFF)) {
		fprintf(stderr, "Error while initializing emulator.\n");
	}

	// Setup palette and LCD mode
	PokeMini_VideoPalette_Init(PokeMini_BGR16, 1);
	PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
	PokeMini_ApplyChanges();

	// Load stuff
	PokeMini_UseDefaultCallbacks();
	if (!PokeMini_LoadFromCommandLines("Using FreeBIOS", "EEPROM data will be discarded!")) {
		UI_Status = UI_STATUS_MENU;
	}

	// Enable sound & init UI
	printf("Starting emulator...\n");
	UIMenu_Init();
	enablesound(CommandLine.sound);

	// Emulator's loop
	unsigned long time, NewTickSync = 0;
	SDL_FillRect(screen, NULL, 0);
	while (emurunning) {
		// Emulate and syncronize
		if (RequireSoundSync) {
			PokeMini_EmulateFrame();
			// Sleep a little in the hope to free a few samples
			while (MinxAudio_SyncWithAudio()) SDL_Delay(1);
		} else {
			time = SDL_GetTicks();
			PokeMini_EmulateFrame();
			do {
				SDL_Delay(1);		// This lower CPU usage
				time = SDL_GetTicks();
			} while (time < NewTickSync);
			NewTickSync = time + 13;	// Aprox 72 times per sec
		}

		// Screen rendering
		if (SDL_LockSurface(screen) == 0) {
			// Render the menu or the game screen
			if (PokeMini_Rumbling) {
				SDL_FillRect(screen, NULL, 0);
				PokeMini_VideoBlit((uint16_t *)screen->pixels + ScOffP + PokeMini_GenRumbleOffset(PixPitch), PixPitch);
			} else {
				PokeMini_VideoBlit((uint16_t *)screen->pixels + ScOffP, PixPitch);
			}
			LCDDirty = 0;
			// Unlock surface
			SDL_UnlockSurface(screen);
			if (CommandLine.scaling == 1)
			{
				bitmap_scale(16, 24, 288, 192, rl_screen->w, rl_screen->h, screen->w, 0, (uint16_t* restrict)screen->pixels, (uint16_t* restrict)rl_screen->pixels);
			}
			else
			{
				SDL_BlitSurface(screen, &rct, rl_screen, &pos);
			}
			SDL_Flip(rl_screen);
		}

		// Handle events
		while (SDL_PollEvent(&event)) handleevents(&event);

		// Menu
		if (UI_Status == UI_STATUS_MENU) {
			enablesound(0);
			if (mmenu) {
				ShowMenu_t ShowMenu = (ShowMenu_t)dlsym(mmenu, "ShowMenu");
				
				MenuReturnStatus status = ShowMenu(CommandLine.min_file, save_path, rl_screen, kMenuEventKeyDown);
				
				UI_Status = UI_STATUS_GAME;
				if (status!=kStatusExitGame) enablesound(CommandLine.sound);

				if (status==kStatusExitGame) {
					emurunning = 0;
				}
				else if (status==kStatusOpenMenu) {
					UI_Status = UI_STATUS_MENU;
					menuloop();
				}
				else if (status>=kStatusLoadSlot) {
					int slot = status - kStatusLoadSlot;
					char tmp[256];
					sprintf(tmp, save_path, slot);
					PokeMini_LoadSSFile(tmp);
				}
				else if (status>=kStatusSaveSlot) {
					int slot = status - kStatusSaveSlot;
					char tmp[256];
					sprintf(tmp, save_path, slot);
					PokeMini_SaveSSFile(tmp, CommandLine.min_file);
				}
			}
			else {
				menuloop();
			}
			Clear_Screen();
		}
	}

	// Disable sound & free UI
	enablesound(0);
	UIMenu_Destroy();
	
	if (rl_screen) SDL_FreeSurface(rl_screen);
	if (screen) SDL_FreeSurface(screen);
	
	// Save Stuff
	PokeMini_SaveFromCommandLines(1);

	// Close joystick
	if (joy) SDL_JoystickClose(joy);

	// Terminate...
	printf("Shutdown emulator...\n");
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();

	if (mmenu) dlclose(mmenu);
	return 0;
}

