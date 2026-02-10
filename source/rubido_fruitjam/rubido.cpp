#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sound.h"
#include "cboardparts.h"
#include "cmainmenu.h"
#include "cpeg.h"
#include "cselector.h"
#include "commonvars.h"
#include "framebuffer.h"
#include "glcdfont.h"
#include "usbh_processor.h"
#include "images/veryeasy1_RGB565_LE.h"
#include "images/veryhard1_RGB565_LE.h"
#include "images/hard1_RGB565_LE.h"
#include "images/easy1_RGB565_LE.h"
#include "images/credits_RGB565_LE.h"
#include "images/infoeasy_RGB565_LE.h"
#include "images/infohard_RGB565_LE.h"
#include "images/infoveryeasy_RGB565_LE.h"
#include "images/infoveryhard_RGB565_LE.h"
#include "images/background_RGB565_LE.h"

// Load the settings, if there isn't a settings file, set some initial values
void LoadSettings()
{
	// SDFile* SettingsFile;
	// SettingsFile = pd->file->open("settings.dat", kFileReadData);
	// if (SettingsFile)
	// {
	// 	pd->file->read(SettingsFile, &BestPegsLeft[VeryEasy], sizeof(int));
	// 	pd->file->read(SettingsFile, &BestPegsLeft[Easy], sizeof(int));
	// 	pd->file->read(SettingsFile, &BestPegsLeft[Hard], sizeof(int));
	// 	pd->file->read(SettingsFile, &BestPegsLeft[VeryHard], sizeof(int));
	// 	int tmp;
	// 	pd->file->read(SettingsFile, &tmp, sizeof(int));
	// 	setSoundOn(tmp);
	// 	pd->file->read(SettingsFile, &tmp, sizeof(int));
	// 	setMusicOn(tmp);
	// 	pd->file->close(SettingsFile);
	// }
	// else
	// {
		BestPegsLeft[VeryEasy] = 0;
		BestPegsLeft[Easy] = 0;
		BestPegsLeft[Hard] = 0;
		BestPegsLeft[VeryHard] = 0;
		setSoundOn(true);
	//}
}

// Save the settings
void SaveSettings()
{
	// SDFile* SettingsFile;
	// SettingsFile = pd->file->open("settings.dat", kFileWrite);
	// if (SettingsFile)
	// {
	// 	pd->file->write(SettingsFile, &BestPegsLeft[VeryEasy], sizeof(int));
	// 	pd->file->write(SettingsFile, &BestPegsLeft[Easy], sizeof(int));
	// 	pd->file->write(SettingsFile, &BestPegsLeft[Hard], sizeof(int));
	// 	pd->file->write(SettingsFile, &BestPegsLeft[VeryHard], sizeof(int));
	// 	int tmp = isSoundOn();
	// 	pd->file->write(SettingsFile, &tmp, sizeof(int));
	// 	tmp = isMusicOn();
	// 	pd->file->write(SettingsFile, &tmp, sizeof(int));
	// 	pd->file->close(SettingsFile);
	// }
}

void resetGlobals()
{
	PrintFormShown = false;
	Difficulty = VeryEasy;
	Moves = 0;
	GameState = GSTitleScreenInit;
}

void setupGame()
{
	resetGlobals();
	initSound();
	LoadSettings();
	BoardParts = CBoardParts_Create();
	Menu = CMainMenu_Create();
	GameSelector = CSelector_Create(4,4);
}

void terminateGame()
{
	CBoardParts_Destroy(BoardParts);
	CMainMenu_Destroy(Menu);
	CSelector_Destroy(GameSelector);
	deInitSound();
	SaveSettings();
}

// procedure that calculates how many moves are possible in the current board state
// we can simply do this by checking all parts and see if they can move to all directions
// the canmoveto method in CPegs is does all the checking
int MovesLeft()
{
    CPeg *BoardPart ;
    int TmpMoves=0,Y,X;
    for (Y=0;Y<NrOfRows;Y++)
        for (X=0;X<NrOfCols;X++)
        {
            BoardPart = CBoardParts_GetPart(BoardParts, X,Y);
            // if there is a boardpart on that X,Y Coordinate
            // check all direction if we can move to that if so increases the movesleft
            if (BoardPart)
            {
                if (CPeg_CanMoveTo(BoardPart,X+2,Y,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X-2,Y,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X,Y-2,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X,Y+2,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X+2,Y-2,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X+2,Y+2,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X-2,Y+2,false))
                    TmpMoves++;
                if (CPeg_CanMoveTo(BoardPart,X-2,Y-2,false))
                    TmpMoves++;
            }
        }
    return TmpMoves;
}

// Procedure that counts how many pegs are left on the board (animphase must be lower then 2 io a red or blue peg)
int PegsLeft()
{
    CPeg *BoardPart ;
    int Pegs=0,Y,X;
    for (Y=0;Y<NrOfRows;Y++)
        for (X=0;X<NrOfCols;X++)
        {
            BoardPart = CBoardParts_GetPart(BoardParts,X,Y);
            if (BoardPart)
                if ((CPeg_GetType(BoardPart) == IDPeg) && (CPeg_GetAnimPhase(BoardPart) < 2))
                    Pegs++;
        }
    return Pegs;
}

// procedure that draws the board, boardparts info and a boxed message over the playfield
// and waits till the A button is pressed

void PrintForm(const char *msg)
{
	PrintFormShown = true;
	bufferFillRect(&fb,3,75,231,160-55,COLOR_BACKGROUND);
	bufferDrawRect(&fb,3,75,231,160-55,COLOR_FOREGROUND);
	bufferDrawRect(&fb,5,77,231-4,160-55-4,COLOR_FOREGROUND);
	bufferPrint(&fb,9,85,msg,COLOR_FOREGROUND,COLOR_FOREGROUND,1,font);
}

// this will ceate the initial board state, io a cross of pegs, with the middle on being empty (=animphase 6)
void InitBoard()
{
	CBoardParts_RemoveAll(BoardParts);
    int X,Y;
    for (Y=0;Y < NrOfRows;Y++)
        for (X=0;X < NrOfCols;X++)
        {
            if ((Y < 3) && (X > 2) && (X < 6))
                CBoardParts_Add(BoardParts,CPeg_Create(X,Y));
             if ((Y > 2) && (Y <6))
                CBoardParts_Add(BoardParts,CPeg_Create(X,Y));
            if ((Y > 5) && (X > 2) && (X <6))
                CBoardParts_Add(BoardParts,CPeg_Create(X,Y));
        }
     CPeg_SetAnimPhase(CBoardParts_GetPart(BoardParts,4,4), 6);

}

// Checks if we won the game
bool IsWinningGame()
{
    if (PegsLeft() == 1) //must be 1 peg left
    {
        if ((Difficulty == VeryHard) || (Difficulty == Easy))
        {
            if(CPeg_GetAnimPhase(CBoardParts_GetPart(BoardParts,4,4)) < 2) // must be in the middle with veryhard or easy
                return true;
            else
                return false;
        }
        else
            return true;
    }
    else
        return false;
}

void GameInit()
{
	CSelector_SetPosition(GameSelector, 4, 4);
	InitBoard();
    Moves = 0;
	playStartSound();
	PrintFormShown = false;
}

// The main Game Loop
void Game()
{
	if (GameState == GSGameInit)
	{
		GameInit();
		GameState -= GSInitDiff;
	}
	bufferDrawImage(&fb,0,0,background_data, background_width, background_height, false, true, false);
	char Msg[100];

	// Write some info to the screen
	sprintf(Msg, "Moves Left:%d", MovesLeft());
	bufferPrint(&fb, 242, 37, Msg, COLOR_FOREGROUND, COLOR_FOREGROUND,1,font);
	
	sprintf(Msg, "Moves:%d", Moves);
	bufferPrint(&fb, 242, 53, Msg, COLOR_FOREGROUND, COLOR_FOREGROUND,1,font);

	sprintf(Msg, "Pegs Left:%d", PegsLeft());
	bufferPrint(&fb, 242, 69, Msg, COLOR_FOREGROUND, COLOR_FOREGROUND,1,font);

	// Only show best pegs if it isn't 0
	if (BestPegsLeft[Difficulty] != 0)
	{
		sprintf(Msg, "Best Pegs:%d", BestPegsLeft[Difficulty]);
		bufferPrint(&fb, 242, 85, Msg, COLOR_FOREGROUND, COLOR_FOREGROUND,1,font);
	}
	CBoardParts_Draw(BoardParts);
	CSelector_Draw(GameSelector);

	//need to repeat showing it until it's no longer shown
	if (PrintFormShown)
	{
		// if it's the winning game play the winning sound and show the form with the winning message
		if (IsWinningGame())
		{
			PrintForm("Congrats you have solved the\npuzzle! Try a new difficulty!\n\nPress (A) to continue");
		}
		else // show the loser messager, play loser sound
		{
			PrintForm("You couldn't solve the puzzle!\nDon't give up, try it again!\n\nPress (A) to continue");
		}
	}

	if(gamepadButtonJustPressed(GAMEPAD_LEFT) || keyJustPressed(LEFTKEY))
		if (!PrintFormShown)
			CSelector_SetPosition(GameSelector, CSelector_GetPosition(GameSelector).X - 1, CSelector_GetPosition(GameSelector).Y);

	if(gamepadButtonJustPressed(GAMEPAD_RIGHT) || keyJustPressed(RIGHTKEY))
		if (!PrintFormShown)
			CSelector_SetPosition(GameSelector, CSelector_GetPosition(GameSelector).X + 1, CSelector_GetPosition(GameSelector).Y);

	if(gamepadButtonJustPressed(GAMEPAD_UP) || keyJustPressed(UPKEY))
		if (!PrintFormShown)
			CSelector_SetPosition(GameSelector, CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y - 1);

	if(gamepadButtonJustPressed(GAMEPAD_DOWN) || keyJustPressed(DOWNKEY))
		if (!PrintFormShown)
			CSelector_SetPosition(GameSelector, CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y + 1);

	if((currButtons & BUTTON_1_MASK) && ! (prevButtons && BUTTON_1_MASK))
	{
		if (!PrintFormShown)
		{
			int x = CSelector_GetPosition(GameSelector).X;
			int y = CSelector_GetPosition(GameSelector).Y;

			x--;			
			int checkx = (y > 5) || (y < 3) ? 3 : 0;						
			if (x < checkx)
			{	
				y--;
				if(y < 0)
					y = NrOfRows - 1;
				if((y > 5) || (y < 3))
					x = 5;
				else
					x = NrOfCols - 1;			

			}
			CSelector_SetPosition(GameSelector, x, y);
		}
	}

	if((currButtons & BUTTON_3_MASK) && ! (prevButtons && BUTTON_3_MASK))
	{
		if (!PrintFormShown)
		{
			int x = CSelector_GetPosition(GameSelector).X;
			int y = CSelector_GetPosition(GameSelector).Y;

			x++;			
			int checkx = (y > 5) || (y < 3) ? 5 : NrOfCols-1;						
			if (x > checkx)
			{	
				y++;
				if(y > NrOfRows-1)
					y = 0;
				if((y > 5) || (y < 3))
					x = 3;
				else
					x = 0;			

			}
			CSelector_SetPosition(GameSelector, x, y);
		}
	}

	if(gamepadButtonJustPressed(GAMEPAD_B) || keyJustPressed(BUTTONB_KEY))
	{
		GameState = GSTitleScreenInit;
		PrintFormShown = false;
		playMenuBackSound();
	}

	if(gamepadButtonJustPressed(GAMEPAD_A) || keyJustPressed(BUTTONA_KEY) || 
		((currButtons & BUTTON_2_MASK) && ! (prevButtons && BUTTON_2_MASK)))
	{
		if (PrintFormShown)
		{
			GameState = GSTitleScreenInit;
			PrintFormShown = false;
			playMenuAcknowlege();
		}
		else
		{
			if (GameSelector->HasSelection) // if we have a selection
			{
				// see if the selected boardpart can move to the current position
				if (CPeg_CanMoveTo(CBoardParts_GetPart(BoardParts, CSelector_GetSelection(GameSelector).X, CSelector_GetSelection(GameSelector).Y), CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y, true))
				{
					playGameAction();
					//if so play a sound, increase the moves, set the selected part to empty and the current part to red
					Moves++;
					CPeg_SetAnimPhase(CBoardParts_GetPart(BoardParts, CSelector_GetSelection(GameSelector).X, CSelector_GetSelection(GameSelector).Y), 6);
					CPeg_SetAnimPhase(CBoardParts_GetPart(BoardParts, CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y), 0);
					// if no moves are left see if the best pegs left value for the current difficulty is
					// greater if so set te new value
					if (MovesLeft() == 0)
					{
						if (BestPegsLeft[Difficulty] != 0)
						{
							if (PegsLeft() < BestPegsLeft[Difficulty])
								BestPegsLeft[Difficulty] = PegsLeft();
						}
						else
							BestPegsLeft[Difficulty] = PegsLeft();
						SaveSettings();
						// if it's the winning game play the winning sound and show the form with the winning message
						if (IsWinningGame())
						{
							playWinnerSound();
							PrintFormShown = true;
						}
						else // show the loser messager, play loser sound
						{
							playLoserSound();
							PrintFormShown = true;
						}
					}

				}
				else // if we can't move to the spot, play the wrong move sound, and reset the selection to a red peg (instead of blue / selected)
				{
					CPeg_SetAnimPhase(CBoardParts_GetPart(BoardParts, CSelector_GetSelection(GameSelector).X, CSelector_GetSelection(GameSelector).Y), 0);
					playErrorSound();
				}
				CSelector_DeSelect(GameSelector); // deselect the selection
			}
			else // we didn't have a selection, set the new selection
			{
				if (CPeg_GetAnimPhase(CBoardParts_GetPart(BoardParts, CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y)) == 0)
				{
					playGameAction();
					CPeg_SetAnimPhase(CBoardParts_GetPart(BoardParts, CSelector_GetPosition(GameSelector).X, CSelector_GetPosition(GameSelector).Y), 1);
					CSelector_Select(GameSelector);
				}

			}
		}
	}
}

void TitleScreenInit()
{

}

// main title screen loop
void TitleScreen()
{
	if(GameState == GSTitleScreenInit)
	{
		TitleScreenInit();
		GameState -= GSInitDiff;
	}

	if(gamepadButtonJustPressed(GAMEPAD_DOWN) || keyJustPressed(DOWNKEY) ||
		((currButtons & BUTTON_3_MASK) && ! (prevButtons && BUTTON_3_MASK)))
	{
		CMainMenu_NextItem(Menu);
	}
	if(gamepadButtonJustPressed(GAMEPAD_UP) || keyJustPressed(UPKEY) ||
		((currButtons & BUTTON_1_MASK) && ! (prevButtons && BUTTON_1_MASK)))
	{
		CMainMenu_PreviousItem(Menu);
	}
	if(gamepadButtonJustPressed(GAMEPAD_A) || keyJustPressed(BUTTONA_KEY) ||
		((currButtons & BUTTON_2_MASK) && ! (prevButtons && BUTTON_2_MASK)))
	{
		playMenuAcknowlege();
		switch (CMainMenu_GetSelection(Menu))
		{
			case 1:
				GameState = GSDifficultySelectInit;
				break;
			case 2:
				GameState = GSCreditsInit;
				break;
		}
	}
	CMainMenu_Draw(Menu);
}

void DifficultySelectInit()
{
}

// Main difficulty select loop
void DifficultySelect()
{
	if(GameState == GSDifficultySelectInit)
	{
		DifficultySelectInit();
		GameState -= GSInitDiff;
	}


	if(gamepadButtonJustPressed(GAMEPAD_B) || keyJustPressed(BUTTONB_KEY))
	{
		GameState = GSTitleScreenInit;
		playMenuBackSound();
	}
	
	if(gamepadButtonJustPressed(GAMEPAD_A) || keyJustPressed(BUTTONA_KEY) ||
		((currButtons & BUTTON_2_MASK) && ! (prevButtons && BUTTON_2_MASK)))
	{
		playMenuAcknowlege();
		GameState = GSGameInit;
	}
	
	if(gamepadButtonJustPressed(GAMEPAD_LEFT) || keyJustPressed(LEFTKEY) ||
		((currButtons & BUTTON_1_MASK) && ! (prevButtons && BUTTON_1_MASK)))
	{
		playMenuSelectSound();
		if (Difficulty == VeryHard)
		{
			Difficulty = Hard;
		}
		else
			if (Difficulty == Hard)
			{
				Difficulty = Easy;
			}
			else
				if (Difficulty == Easy)
				{
					Difficulty = VeryEasy;
				}
				else
					if (Difficulty == VeryEasy)
						Difficulty = VeryHard;
	}
	
	if(gamepadButtonJustPressed(GAMEPAD_RIGHT) || keyJustPressed(RIGHTKEY) ||
		((currButtons & BUTTON_3_MASK) && ! (prevButtons && BUTTON_3_MASK)))
	{
		playMenuSelectSound();
		if (Difficulty == VeryEasy)
		{
			Difficulty = Easy;
		}
		else
			if (Difficulty == Easy)
			{
				Difficulty = Hard;
			}
			else
				if (Difficulty == Hard)
				{
					Difficulty = VeryHard;
				}
				else
					if (Difficulty == VeryHard)
						Difficulty = VeryEasy;
	}
	
	// decide what we draw to the buffer based on the difficuly
	switch(Difficulty)
	{
		case VeryEasy:
			bufferDrawImage(&fb, 0, 0, infoveryeasy_data, infoveryeasy_width, infoveryeasy_height, false, true, false);
			bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - veryeasy1_width / 2, 35, veryeasy1_data, veryeasy1_width, veryeasy1_height, false, true, false,COLOR_TRANSPARENT);
			break;
		case Easy:
			bufferDrawImage(&fb, 0, 0, infoeasy_data, infoeasy_width, infoeasy_height, false, true, false);
			bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - easy1_width / 2, 35, easy1_data, easy1_width, easy1_height, false, true, false,COLOR_TRANSPARENT);
		break;
		case Hard:
			bufferDrawImage(&fb, 0, 0, infohard_data, infohard_width, infohard_height, false, true, false);
			bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - hard1_width / 2, 35, hard1_data, hard1_width, hard1_height, false, true, false,COLOR_TRANSPARENT);
			break;
		case VeryHard:
			bufferDrawImage(&fb, 0, 0, infoveryhard_data, infoveryhard_width, infoveryhard_height, false, true, false);
			bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - veryhard1_width / 2, 35, veryhard1_data, veryhard1_width, veryhard1_height, false, true, false,COLOR_TRANSPARENT);
			break;

	}
}

void CreditsInit()
{
}

//Main Credits loop, will just show an image and wait for a button to be pressed
void Credits()
{
	if (GameState == GSCreditsInit)
	{
		CreditsInit();
		GameState -= GSInitDiff;
	}

	if (gamepadButtonJustPressed(GAMEPAD_A) || keyJustPressed(BUTTONA_KEY) ||
		gamepadButtonJustPressed(GAMEPAD_B) || keyJustPressed(BUTTONB_KEY) ||
		((currButtons & BUTTON_2_MASK) && ! (prevButtons && BUTTON_2_MASK)))
		{
			playMenuAcknowlege();
			GameState = GSTitleScreenInit;
		}
	bufferDrawImage(&fb,0,0,credits_data, credits_width, credits_height, false, true, false);
}

void mainLoop(void)
{
    switch(GameState)
    {
		case GSGameInit:
        case GSGame :
            Game();
            break;
		case GSTitleScreenInit:
        case GSTitleScreen:
            TitleScreen();
            break;
		case GSDifficultySelectInit:
        case GSDifficultySelect:
            DifficultySelect();
            break;
		case GSCreditsInit:
        case GSCredits:
            Credits();
            break;
		default:
			break;
    }
}
