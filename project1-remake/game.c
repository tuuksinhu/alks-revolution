/*
 * The different colors, such as PRETTYGREEN and ALMOSTBLACK
 * are all defined in "../data/useful.h", which is ported 
 * from "game.hpp".
 *
 *
 *
 *
 *
 *
 *
*/

// including main library with everything
#include "game.hpp"

// stuff
float FLOAT_WW = 1366.0f;
float FLOAT_WH = 768.0f;
int INT_WW = 1366;
int INT_WH = 768;

// enums
typedef enum GameStyle
{
    PLATFORMER,
    NOTPLATFORMER
} GameStyle;

typedef enum Screen
{
    TITLE,
    BASE,
    HELP
} Screen;

typedef enum ActualWeapon
{
    NONE,
    SCYTHE,
    WAND
} ActualWeapon;

    // this should not work like that. i will
    // leave it as it is but at some point it
    // will be useless
typedef enum Windowstate
{
    CLOSED,
    OPENED
} Windowstate;

// main definitions

    // enum
GameStyle mainGameStyle;

Screen currentScreen;

ActualWeapon mainActualWeapon;

Windowstate mainWindowState;

    // texture
Texture2D grassSprite;

Texture2D playerSprite;

Texture2D rubySprite;

Texture2D sword1Sprite;
Texture2D wandSprite;

    // font
Font romulusFont;

    // camera
Camera2D baseCamera;

// structs
struct Player player =
{
    .Rotation = 0.00,
    .Scale = 1.00,

    .Speed = 7,
    .Dash = 75,
    .Gravity = 4,
    .Jump = 15,

    .Width = 15,
    .Height = 15
};

struct Vector2 playerPos =
{
    .x = FLOAT_WW/2 - 15,
    .y = FLOAT_WH/2 - 15
};

struct Weapon sword1 =
{
    .Rotation = 0.00,
    .Scale = 1.00,

    .Movement = 2,

    .Width = 32,
    .Height = 32
};

struct Weapon wand =
{
    .Rotation = 0.00,
    .Scale = 1.00,

    .Movement = 2,

    .Width = 32,
    .Height = 32
};

struct Gem gem =
{
    .Rotation = 0,
    .Scale = 1
};

struct Vector2 gemPos =
{
    .x = static_cast<float>(rand() % INT_WW),
    .y = static_cast<float>(rand() % INT_WH)
};

// functions
void MousePosition()
{
    Vector2 mousePos;
    mousePos = GetMousePosition();

    if (IsKeyPressed(KEY_M))
    {
        std::cout << "\ny: " << mousePos.y << std::endl;
        std::cout << "\nx: " << mousePos.x << std::endl;
        std::cout << "\n\n" << std::endl;
        // aids::println(stdout, "\n\n\n\n" sdakdjaskdjakldjkdjaslk);
    }
}

void PlatformerPlayerMovimentation()
{
    int playerHeightDistanceToWindowLowestPoint = 10;

    playerPos.y += player.Gravity;

    if (playerPos.y >= FLOAT_WH - player.Height - playerHeightDistanceToWindowLowestPoint)
    {
        player.Gravity = 0;
    }

    // key movimentation
    if (IsKeyPressed(KEY_SPACE))
    {
        player.Gravity -= player.Jump;
    }

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        playerPos.x -= player.Speed;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        playerPos.x += player.Speed;
    }

    if (IsKeyPressed(KEY_R))
    {
        playerPos.x = FLOAT_WW/2 - 15;
        playerPos.y = FLOAT_WH/2 - 15;
    }
}

void NotPlatformerPlayerMovimentation()
{
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            playerPos.y -= player.Dash;
        }

        playerPos.y -= player.Speed;
    }

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            playerPos.y += player.Dash;
        }

        playerPos.y += player.Speed;
    }

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            playerPos.x -= player.Dash;
        }

        playerPos.x -= player.Speed;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            playerPos.x += player.Dash;
        }

        playerPos.x += player.Speed;
    }

    if (IsKeyPressed(KEY_R))
    {
        playerPos.x = FLOAT_WW/2 - 15;
        playerPos.y = FLOAT_WH/2 - 15;
    }
}

void sword1Function()
{
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        sword1.Rotation = 4;
    }
    else
    {
        sword1.Rotation = 0;
    }
}

void wandFunction()
{
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        wand.Rotation = 4;
    }
    else
    {
        wand.Rotation = 0;
    }
}

void BothGameStylesFunction()
{
    MousePosition();
    
    baseCamera.offset = ( Vector2 ) { FLOAT_WW/2, FLOAT_WH/2 };
    baseCamera.target = ( Vector2 ) { playerPos.x - (player.Width/2), playerPos.y - (player.Height/2) };
    baseCamera.rotation = 0.0f;
    baseCamera.zoom = 1.0f;

    // to the arsenal
    if (IsKeyPressed(KEY_ONE))
    {
        mainActualWeapon = NONE;
    }

    if (IsKeyPressed(KEY_TWO))
    {
        mainActualWeapon = SCYTHE;
    }

    if (IsKeyPressed(KEY_THREE))
    {
        mainActualWeapon = WAND;
    }

    // player contact with gem
    if (playerPos.y == gemPos.y && playerPos.x == gemPos.x)
    {
        playerPos.x = 1;
    }

    // to title
    if (IsKeyPressed(KEY_F1))
    {
        currentScreen = TITLE;
    }
}

void MainTextureDrawing()
{
    if (IsKeyDown(KEY_F2))
    {
        DrawFPS(FLOAT_WW - 80, 4);
    }

    DrawTextureEx(playerSprite, playerPos, player.Rotation, player.Scale, WHITE);
    
    DrawTextureEx(rubySprite, gemPos, gem.Rotation, gem.Scale, WHITE);
 
    // implement the collision player-gem
    // try using the base:
        /*
        if (playerPos.x >= gemPos.x && playerPos.x <= gemPos.x + player.Width \
            && playerPos.y >= gemPos.y && playerPos.y <= playerPos.y + player.Height)
        {
            DrawTextureEx(rubySprite, gemPos, gem.Rotation, gem.Scale, WHITE);
        }
        */


    // the best ever method of implementing
    // an arsenal of weapons. simple and
    // easy to use
    switch (mainActualWeapon)
    {
        case NONE:
        {

        }
        break;

        case SCYTHE:
        {
            sword1Function();

            struct Vector2 sword1Pos =
            {
                .x = playerPos.x - player.Width  - 1,
                .y = playerPos.y - player.Height - 1
            };

            DrawTextureEx(sword1Sprite, sword1Pos, sword1.Rotation, sword1.Scale, WHITE);
        }
        break;

        case WAND:
        {
            wandFunction();

            struct Vector2 wandPos =
            {
                .x = playerPos.x - player.Width  - 1,
                .y = playerPos.y - player.Height - 1
            };

            DrawTextureEx(wandSprite, wandPos, wand.Rotation, wand.Scale, WHITE);
        }
        break;
    }
}

void BasePlatformerFunction()
{
    BothGameStylesFunction();

    PlatformerPlayerMovimentation();

    BeginMode2D(baseCamera);
    BeginDrawing();
    ClearBackground(PRETTYGREEN);

        MainTextureDrawing();

    EndDrawing();
    EndMode2D();
    
    if (IsKeyPressed(KEY_N))
    {
        mainGameStyle = NOTPLATFORMER;
    }
}

void BaseNotPlatformerFunction()
{
    BothGameStylesFunction();

    NotPlatformerPlayerMovimentation();
    
    BeginMode2D(baseCamera);
    BeginDrawing();
    ClearBackground(PRETTYGREEN);

        MainTextureDrawing();

    EndDrawing();
    EndMode2D();

    if (IsKeyPressed(KEY_P))
    {
        mainGameStyle = PLATFORMER;
    }
}

void TitleFunction()
{
    Vector2 mousePos;
    mousePos = GetMousePosition();

    MousePosition();

    Vector2 PlatPlayPos;
    PlatPlayPos.x = FLOAT_WW/2;
    PlatPlayPos.y = FLOAT_WH/2;

    Vector2 helpPos;
    helpPos.x = FLOAT_WW/2;
    helpPos.y = FLOAT_WH/2 + 20;

    BeginDrawing();
    ClearBackground(BLACK);

        DrawTextEx(romulusFont, "Title Screen!", ( Vector2 ) { 20, 10 }, 50, 4, WHITE);
        DrawTextEx(romulusFont, "See help before playing!!!", ( Vector2 ) { FLOAT_WW - 300, FLOAT_WH - 25 }, 15, 4, WHITE);

        // implement something to change modes
        // between platfromer and not-platformer
            // focus on putting a simple button
            // that when pressed it changes the
            // mode.
                // try putting it as a DrawTextEx
                // function and implement the 
                // button just like the "play"
                // is implemented
        DrawTextEx(romulusFont, "PLAY", PlatPlayPos, 30, 4, WHITE);

        DrawTextEx(romulusFont, "HELP", helpPos, 30, 4, WHITE);

        // implement the close button just like
        // the PLAY and HELP are implemented 
            // use the base of the next line    
                // DrawTextEx(romulusFont, "X", closebuttonPos, 50, 0, WHITE);

    EndDrawing();

    // to-plat configuration
    int PlatPlayXplus = PlatPlayPos.x;
    int PlatPlayXminus = PlatPlayPos.x + 81;
    int PlatPlayYplus = PlatPlayPos.y;
    int PlatPlayYminus = PlatPlayPos.y + 20;

    if (mousePos.x >= PlatPlayXplus && mousePos.x <= PlatPlayXminus && mousePos.y
    >= PlatPlayYplus && mousePos.y <= PlatPlayYminus)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentScreen = BASE;
            mainGameStyle = PLATFORMER;
        }
    }

    // to plat configuration
    if (IsKeyPressed(KEY_P))
    {
        currentScreen = BASE;
        mainGameStyle = PLATFORMER;
    }

    // to-notplat configuration
    if (IsKeyPressed(KEY_N))
    {
        currentScreen = BASE;
        mainGameStyle = NOTPLATFORMER;
    }

    // to-HELP configuration
    int helpXplus = helpPos.x;
    int helpXminus = helpPos.x + 84;
    int helpYplus = helpPos.y;
    int helpYminus = helpPos.y + 20;

    if (mousePos.x >= helpXplus && mousePos.x <= helpXminus && mousePos.y
    >= helpYplus && mousePos.y <= helpYminus)
    {
         if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
         {
            currentScreen = HELP;
         }
    }

    if (IsKeyPressed(KEY_H))
    {
        currentScreen = HELP;
    }

    // closebutton configuration
    /*
    int closebuttonXplus = closebuttonPos.x;
    int closebuttonXminus = closebuttonPos.x + 35;
    int closebuttonYplus = closebuttonPos.y;
    int closebuttonYminus = closebuttonPos.y + 50;

    if (mousePos.x >= closebuttonXplus && mousePos.x <= closebuttonXminus && mousePos.y >= closebuttonYplus && mousePos.y <= closebuttonYminus)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            mainWindowState = CLOSED;
        }
    }
    */
}

void HelpFunction()
{
    BeginDrawing();
    ClearBackground(BLACK);

        DrawTextEx(romulusFont, "This is the help page", ( Vector2 ) { 20, 10 }, 50, 4, WHITE);

    EndDrawing();

    if (IsKeyPressed(KEY_F1))
    {
        currentScreen = TITLE;
    }
}

void WindowClose();

// GameLoop
void GameLoop()
{
    mainWindowState = OPENED;
    
    // this is the worst switch you
    // will ever see in your life
    // and I dont care about it.    
    switch (mainWindowState)
    {
        case OPENED:
        {
            while (!WindowShouldClose())
            {
                switch (currentScreen)
                {
                    case BASE:
                    {
                        switch (mainGameStyle)
                        {
                            case PLATFORMER:
                            {
                                BasePlatformerFunction();
                            }
                            break;

                            case NOTPLATFORMER:
                            {
                                BaseNotPlatformerFunction();
                            }
                            break;
                        }
                    }
                    break;

                    case TITLE:
                    {
                        TitleFunction();
                    }
                    break;

                    case HELP:
                    {
                        HelpFunction();
                    }
                    break;
                }
            }
        }
        break;

        case CLOSED:
        {
            WindowClose();
        }
        break;
    }
}

