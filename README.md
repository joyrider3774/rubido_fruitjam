# Rubido Adafruit's fruitjam Version
![DownloadCountTotal](https://img.shields.io/github/downloads/joyrider3774/rubido_fruitjam/total?label=total%20downloads&style=plastic) ![DownloadCountLatest](https://img.shields.io/github/downloads/joyrider3774/rubido_fruitjam/latest/total?style=plastic) ![LatestVersion](https://img.shields.io/github/v/tag/joyrider3774/rubido_fruitjam?label=Latest%20version&style=plastic) ![License](https://img.shields.io/github/license/joyrider3774/rubido_fruitjam?style=plastic)

Rubido is a little chinese checkers or solitaire game with four difficulties.

## Playing the Game:
The aim of the game in chinese checkers is to select a (white) peg on the board and jump over another (white) peg to land on an empty (black) spot. When doing this the peg you jumped over will be removed from the board.
You need to play the game in such a way that only one peg remains on the board at the end. Depending on the difficulty you had chosen this can be either (only) in the middle of the board or anywhere on the board.
Also depending on the difficulty you had chosen you can either jump horizontally and veritically over pegs or diagonally as well.

## Diffuclties 

### Very Easy
- Jump over Pegs vertically, horizontally and diagonally
- Last Peg can be anywhere on the board

### Easy
- Jump over Pegs vertically, horizontally and diagonally
- Last Peg must end on the middle board

### Hard
- Jump over Pegs vertically and horizontally only
- Last Peg can be anywhere on the board

### Very Hard
- Jump over Pegs vertically and horizontally only
- Last Peg must end on the middle board

## Controls USB KeyBoard

| Button | Action |
| ------ | ------ |
| LEFT KEY | Left in difficulties screen. During gameplay move the peg selector left. |
| RIGHT KEY | Right in difficulties screen. During gameplay move the peg selector Right. |
| UP KEY | Up in main menu screen. During gameplay move the peg selector Up. |
| DOWN KEY | Down in main menu screen. During gameplay move the peg selector Down. |
| SPACE KEY | Confirm in menu and difficulty selector. During gameplay activate the peg where the peg selector is. If there was a peg already selected it will deselect it |
| ESC KEY | return to titlescreen |
| F1 KEY | Decrease Volume |
| F2 KEY | Increase Volume |

## Controls Adafruit's SNES Controller

| Button | Action |
| ------ | ------ |
| LEFT DPAD | Left in difficulties screen. During gameplay move the peg selector left. |
| RIGHT DPAD | Right in difficulties screen. During gameplay move the peg selector Right. |
| UP DPAD | Up in main menu screen. During gameplay move the peg selector Up. |
| DOWN DPAD | Down in main menu screen. During gameplay move the peg selector Down. |
| A | Confirm in menu and difficulty selector. During gameplay activate the peg where the peg selector is. If there was a peg already selected it will deselect it |
| B | return to titlescreen |
| LEFT SHOULDER | Decrease Volume |
| RIGHT SHOULDER | Increase Volume |

## Libraries / tools used or required
- **Arduino IDE**: for compiling
- **Adafruit_dvhstx**: for video output
- **Adafruit_TinyUSB**: for usbhost mode for keyboard, joypad and mouse input
- **Adafruit_TLV320DAC3100**: for I2S sound output
- **PICO PIO USB:** for usbhost stuff

## Credits
- i2sTones.cpp: I2S tones library mainly made with the help of [claude.ai](https://claude.ai)
- framebuffer.cpp: rgb565 framebuffer library made with the help of [claude.ai](https://claude.ai)
- glcdfont.h: Adafruit_GFX font mainly used internally to display debug information on an internal buffer
- Graphcis are made by me willems davy aka joyrider3774 using gimp

## Notes
- There are 20 volume levels, the default is set to a low value (being 3) out of safety just in case people use headphones immediatly. you can change the default on this [line](https://github.com/joyrider3774/formula1_fruitjam/blob/main/source/rubido_fruitjam/sound.cpp#L12) if you want higher volume
- Adafruit snes gamepad input controls should work
- Please use arduino-pico 5.5.0 or newer board setup, it seems to have fixed usbhost disconnects