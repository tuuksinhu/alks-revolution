#ifndef __DATA_H__
#define __DATA_H__

#include "raylib.h"

extern Texture2D grassSprite;
extern Texture2D playerSprite;
extern Texture2D rubySprite;
extern Texture2D sword1Sprite;
extern Texture2D wandSprite;

extern Font romulusFont;

extern void LoadTextures();
extern void UnloadTextures();

extern void LoadFonts();
extern void UnloadFonts();

#endif // __DATA_H__
