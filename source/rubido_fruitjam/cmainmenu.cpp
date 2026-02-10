#include "cmainmenu.h"
#include "commonvars.h"
#include "sound.h"
#include "images/titlescreen_RGB565_LE.h"
#include "images/newgame1_RGB565_LE.h"
#include "images/newgame2_RGB565_LE.h"
#include "images/credits1_RGB565_LE.h"
#include "images/credits2_RGB565_LE.h"

CMainMenu* CMainMenu_Create()
{
	CMainMenu* Result = (CMainMenu*)malloc(sizeof(CMainMenu));
    Result->Selection = 1;

	return Result;
}

 // return the current selection
int CMainMenu_GetSelection(CMainMenu* MainMenu)
{
	return MainMenu->Selection;
}

// Destructor will free the surface images
void CMainMenu_Destroy(CMainMenu* MainMenu)
{
    free(MainMenu);
    MainMenu = NULL;
}

// Increase the selection if it goes to far set i to the first selection
void CMainMenu_NextItem(CMainMenu* MainMenu)
{
    MainMenu->Selection++;
    if (MainMenu->Selection == 3)
        MainMenu->Selection = 1;
    
    playMenuSelectSound();
}

// decrease the selection if it goes to low set it to the last selection
void CMainMenu_PreviousItem(CMainMenu* MainMenu)
{
    MainMenu->Selection--;
    if (MainMenu->Selection == 0)
        MainMenu->Selection = 2;
    playMenuSelectSound();
}

// Draw the main menu
void CMainMenu_Draw(CMainMenu* MainMenu)
{
    // draw the title screen background
    bufferDrawImage(&fb,0,0,titlescreen_data,titlescreen_width,titlescreen_height,false,true,false);

    // if selection = newgame draw the purple new game image in the middle of the screen
    if (MainMenu->Selection == 1)
    {
        bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - newgame1_width / 2, 87,newgame1_data, newgame1_width, newgame1_height, false, true, false,COLOR_TRANSPARENT) ;
    }
    else
    // draw the blue one
    {
        bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - newgame2_width / 2, 87,newgame2_data, newgame2_width, newgame2_height, false, true, false,COLOR_TRANSPARENT) ;
    }

    if (MainMenu->Selection == 2) // = credits draw purpe
    {
        bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - credits1_width / 2, 145,credits1_data, credits1_width, credits1_height, false, true, false,COLOR_TRANSPARENT) ;
    }
    else // blue
    {
        bufferDrawImageTransparent(&fb, WINDOW_WIDTH / 2 - credits2_width / 2, 145,credits2_data, credits2_width, credits2_height, false, true, false,COLOR_TRANSPARENT) ;
    }
}
