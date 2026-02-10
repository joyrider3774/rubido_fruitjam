#include <stdint.h>
#include <Adafruit_dvhstx.h>
#include <Adafruit_TinyUSB.h>
#include "commonvars.h"
#include "framebuffer.h"
#include "commonvars.h"
#include "cboardparts.h"
#include "cselector.h"
#include "cmainmenu.h"


#if defined(ADAFRUIT_FEATHER_RP2350_HSTX)
DVHSTXPinout pinConfig = ADAFRUIT_FEATHER_RP2350_CFG;
#elif defined(ADAFRUIT_METRO_RP2350)
DVHSTXPinout pinConfig = ADAFRUIT_METRO_RP2350_CFG;
#elif defined(ARDUINO_ADAFRUIT_FRUITJAM_RP2350)
DVHSTXPinout pinConfig = ADAFRUIT_FRUIT_JAM_CFG;
#elif (defined(ARDUINO_RASPBERRY_PI_PICO_2) || defined(ARDUINO_RASPBERRY_PI_PICO_2W))
DVHSTXPinout pinConfig = ADAFRUIT_HSTXDVIBELL_CFG;
#else
// If your board definition has PIN_CKP and related defines,
// DVHSTX_PINOUT_DEFAULT is available
DVHSTXPinout pinConfig = DVHSTX_PINOUT_DEFAULT;
#endif
Framebuffer fb;
DVHSTX16 tft(pinConfig, DVHSTX_RESOLUTION_320x240, true);

//game
CSelector *GameSelector;
bool PrintFormShown = false;
CBoardParts* BoardParts; // boardparts instance that will hold all the boardparts
int Difficulty = VeryEasy;
int Moves = 0;
int BestPegsLeft[4]; // array that holds the best amount of pegs left for each difficulty

//titlescreen
CMainMenu* Menu;

//main
uint8_t prevButtons, currButtons;
unsigned int prevLogTime;
unsigned int FrameTime, Frames;
bool debugMode = false;
int GameState = GSTitleScreenInit; // the game state

