#ifndef __USEFUL_H__
#define __USEFUL_H__

#ifndef __COLORS_H__
#define __COLORS_H__

#define ALMOSTBLACK    CLITERAL(Color) {15,  15,  15,  255}
#define ALMOSTRAYWHITE CLITERAL(Color) {246, 246, 246, 255}
#define PRETTYGREEN    CLITERAL(Color) {147, 211, 196, 255}

#endif // __COLORS_H__

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

typedef struct Player
{
    float Rotation;
    float Scale;

    int Speed;
    int Dash;
    int Gravity;
    int Jump;

    int Width;
    int Height;
} Player;

typedef struct Weapon
{
    float Rotation;
    float Scale;

    int Movement;

    int Width;
    int Height;
} Weapon;

typedef struct Platform
{
    int x;
    int y;
    int w;
    int h;
} Platform;

typedef struct Background
{
    float Rotation;
    float Scale;
} Background;

typedef struct Gem
{
    float Rotation;
    float Scale;
} Gem;

#endif // __STRUCTS_H__

#endif // __USEFUL_H__
