/*
* Dev: tuuksinhu;
*/

#include "../main/game.hpp"

static int WW = 1366;
static int WH = 768;

// function from main.c
void WindowDraw()
{
    InitWindow(WW, WH, "Test");
    SetExitKey(0);
    SetTargetFPS(60);   
}

// functions ported from data.c
void LoadTextures();
void UnloadTextures();

void LoadFonts();
void UnloadFonts();

// functions ported from game.c
void GameLoop();

void WindowClose()
{
    CloseWindow();
}

// initializing everything
int main()
{
    // initialize the window
    WindowDraw();

    // loading resources (ported from data.c)
    LoadTextures();
    
    // loading fonts (ported from data.c)
    LoadFonts();
    
    // when initializes
    std::cout << "\n\nHello, World!\n\n" << std::endl;
    
    // GameLoop (ported from game.c)
    GameLoop();

    // unloading resources (ported from data.c)
    UnloadTextures();
    
    // unloading fonts (ported from data.c)
    UnloadFonts();

    // close the window
    WindowClose();

    // when closes
    std::cout << "\n\nProgram Closed\n\n" << std::endl;
    
    return 0;
}

