#ifndef COMMONVARS_H
#define COMMONVARS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <Adafruit_dvhstx.h>
#include <Adafruit_TinyUSB.h>
#include "framebuffer.h"
#include "cselector.h"
#include "cmainmenu.h"

typedef struct CBoardParts CBoardParts;
typedef struct CPeg CPeg;
typedef struct SPoint SPoint;
typedef struct CSelector CSelector;



// The diffrent difficultys
#define VeryEasy 0
#define Easy 1
#define Hard 2
#define VeryHard 3

// The diffrent gameStates possible in the game
#define GSQuit 0
#define GSGame 1 
#define GSTitleScreen 2
#define GSDifficultySelect 3 
#define GSCredits 4

#define GSInitDiff 50

#define GSGameInit (GSGame + GSInitDiff)
#define GSTitleScreenInit (GSTitleScreen + GSInitDiff)
#define GSDifficultySelectInit (GSDifficultySelect + GSInitDiff)
#define GSCreditsInit (GSCredits + GSInitDiff)

// window size
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

//game defines
#define NrOfRows 9
#define NrOfCols 9
#define TileWidth 24
#define TileHeight 24
#define IDPeg 1
#define XOffSet 10
#define YOffSet 11

#define RIGHTKEY 0x4F
#define LEFTKEY 0x50
#define DOWNKEY 0x51
#define UPKEY  0x52

#define F1KEY 0x3A
#define F2KEY 0x3B
#define DKEY 0x07


#define BUTTONA_KEY 0x2C //key SPACE
#define BUTTONB_KEY 0x29 //key ESC


#define BUTTON_1_PIN 0
#define BUTTON_2_PIN 4
#define BUTTON_3_PIN 5

#define BUTTON_1_MASK (1 << 0)
#define BUTTON_2_MASK (1 << 1)
#define BUTTON_3_MASK (1 << 2)

#define SAMPLERATE 44100
#define AUDIO_BUFFER_SIZE 4096
#define PIN_USB_HOST_VBUS (11u)
#define FPS 60

#define COLOR_BACKGROUND rgb565(100,120,255)
#define COLOR_FOREGROUND rgb565(0,10,255)
#define COLOR_TRANSPARENT rgb565(0,11,255)

#define CORE1_STACK_SIZE (8 * 1024)  // 8KB instead of default 4KB

extern Framebuffer fb;
extern DVHSTX16 tft;

//game
extern CSelector *GameSelector;
extern bool PrintFormShown;
extern int Moves;
extern int BestPegsLeft[4]; // array that holds the best amount of pegs left for each difficulty
extern int Difficulty;
extern CBoardParts* BoardParts; // boardparts instance that will hold all the boardparts

//titlescreen
extern CMainMenu* Menu;

//main
extern uint8_t prevButtons, currButtons;
extern unsigned int prevLogTime;
extern unsigned int FrameTime, Frames;
extern bool debugMode;
extern int GameState; // the game state

#endif