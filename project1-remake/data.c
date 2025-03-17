#include "../main/game.hpp"

void LoadTextures()
{
    grassSprite = LoadTexture("res/sproutlands/tilesets/Grass.png");

    playerSprite = LoadTexture("res/mine/cube1.png");

    sword1Sprite = LoadTexture("res/weapons/scythe.png");
    wandSprite = LoadTexture("res/weapons/wand.png");

    rubySprite = LoadTexture("res/mine/ruby.png");    
}

void UnloadTextures()
{
    UnloadTexture(grassSprite);

    UnloadTexture(playerSprite);

    UnloadTexture(sword1Sprite);
    UnloadTexture(wandSprite);

    UnloadTexture(rubySprite);
}

void LoadFonts()
{
    romulusFont = LoadFont("res/fonts/romulus.png");
}

void UnloadFonts()
{
    UnloadFont(romulusFont);
}

