#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define WW 800
#define WH 600
#define HALF_WW 385
#define HALF_WH 285

#define ALKS_COUNT 3

#define MAX_BULLETS 10

#define PLAYER_HEALTH_COUNT 5
#define ALKS_HEALTH_COUNT 3

#define MAX_TEXT_SIZE 1024

typedef struct Bullet {
    Vector2 Position;
    Vector2 Direction;
    Vector2 Velocity;

    bool Active;
} Bullet;

typedef struct Enemy {
    float Speed;

    Color *pixels;

    int health;

    int width, height;

    Texture2D Sprite;

    Vector2 Position;

    bool Alive;
} Enemy;

typedef struct Player {
    float Rotation;
    float Scale;

    int Speed;
    int Dash;
    float Run;

    Vector2 Position;

    int health;
} Player;

struct Player player = {
    .Rotation = 1,

    .Speed = 4,
    .Dash = 75,
    .Run = 7.5,

    .Position = (Vector2) { HALF_WW, HALF_WH },

    .health = PLAYER_HEALTH_COUNT
};

Color transparentRaywhite = (Color){245, 245, 245, 128};

Vector2 rubyPosition;

Bullet bullets[MAX_BULLETS];

Enemy alks[ALKS_COUNT];

Texture2D playerSprite;
Texture2D rubySprite;
Texture2D alksSprite;

Font romulusFont;
Font defaultFont;

int i;

char text[MAX_TEXT_SIZE] = { 0 };
int textLength = 0;

void initAlks() {
    srand(time(NULL));
    Image alksImage = LoadImageFromTexture(alksSprite);
    Color *alksPixels = LoadImageColors(alksImage);

    for (int i = 0; i < ALKS_COUNT; i++) {
        alks[i].Position.x = rand() % (WW - alksSprite.width);
        alks[i].Position.y = rand() % WH;
        alks[i].Speed = (rand() % 3) + 1.5f;
        alks[i].pixels = alksPixels;
        alks[i].width = alksSprite.width;
        alks[i].height = alksSprite.height;
        alks[i].health = ALKS_HEALTH_COUNT;
    }

    UnloadImage(alksImage);
}

void respawnAlks(int index) {
    alks[index].Position.x = rand() % (WW - alksSprite.width);
    alks[index].Position.y = rand() % WH;
    alks[index].health = 3;
}

Vector2 GetPlayerDirection() {
    Vector2 Dir = { 0, 0 };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) Dir.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) Dir.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) Dir.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_D)) Dir.x += 1;

    if (Dir.x != 0 || Dir.y != 0) {
        float length = sqrt(Dir.x * Dir.x + Dir.y * Dir.y);
        Dir.x /= length;
        Dir.y /= length;
    }

    return Dir;
}

Vector2 lastDirection = { 1, 0 };

bool CheckCollisionBulletAndEnemy(Bullet bullet, Enemy *alks) {
    Rectangle bulletRect = { bullet.Position.x, bullet.Position.y, 5, 5 };
    Rectangle alksRect = { alks->Position.x, alks->Position.y, alks->width, alks->height };

    return CheckCollisionRecs(bulletRect, alksRect);
}

void initBullets() {
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].Active = false;
    }
}

void shootBullets() {
    Vector2 moveDir = GetPlayerDirection();
    if (moveDir.x != 0 || moveDir.y != 0) {
        lastDirection = moveDir;
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].Active) {
            bullets[i].Position = player.Position;
            bullets[i].Velocity.x = lastDirection.x * 10;
            bullets[i].Velocity.y = lastDirection.y * 10;
            bullets[i].Active = true;
            break;
        }
    }
}

void updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].Active) {
            bullets[i].Position.x += bullets[i].Velocity.x;
            bullets[i].Position.y += bullets[i].Velocity.y;

            if (bullets[i].Position.x < 0 || bullets[i].Position.x > WW ||
                bullets[i].Position.y < 0 || bullets[i].Position.y > WH) {
                bullets[i].Active = false;
            }
        }

        for (int j = 0; j < ALKS_COUNT; j++) {
            if (alks[j].Position.x != -999) {
                if (CheckCollisionBulletAndEnemy(bullets[i], &alks[j])) {
                    bullets[i].Active = false;
                    alks[j].health--;
                    if (alks[j].health <= 0) {
                        respawnAlks(j);
                    }
                    break;
                }
            }
        }
    }
}

void drawAlksHealthBar(Enemy *alks) {
    Rectangle healthBar = { alks->Position.x, alks->Position.y - 10, 30, 5 };
    Color healthColor = RED;

    if (alks->health == 2) healthColor = YELLOW;
    if (alks->health == 3) healthColor = GREEN;

    DrawRectangle(healthBar.x, healthBar.y, healthBar.width, healthBar.height, GRAY);
    DrawRectangle(healthBar.x, healthBar.y, (healthBar.width * alks->health) / 3, healthBar.height, healthColor);
}

void drawPlayerHealthBar(Player *player) {
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barY = 10;

    int currentWidth = (player->health * barWidth) / 5;

    DrawRectangle(barX, barY, barWidth, barHeight, GRAY);

    Color healthColor = (player->health > 4) ? GREEN : (player->health > 2) ? YELLOW : RED;
    DrawRectangle(barX, barY, currentWidth, barHeight, healthColor);

    char healthText[10];
    sprintf(healthText, "%d", player->health);
    DrawText(healthText, barX + barWidth + 10, barY, 20, BLACK);

}

Color GetStoredPixelColor(Color *pixels, int width, int height, int x, int y) {
    if (x<0 || y < 0 || x>= width || y >= height) {
        return BLANK;
    }

    return pixels[y * width + x];
}

bool CheckPixelCollisionPlayerAndEnemy(Enemy alks, Color *playerPixels, int playerWidth, int playerHeight) {
    for (int y = 0; y < alks.height; y++) {
        for (int x = 0; x < alks.width; x++) {
            Vector2 alksWorldPos = { alks.Position.x + x, alks.Position.y + y};
            Vector2 playerLocalPos = { alksWorldPos.x - player.Position.x, alksWorldPos.y - player.Position.y };

            if (playerLocalPos.x >= 0 && playerLocalPos.x < playerWidth && 
                playerLocalPos.y >= 0 && playerLocalPos.y < playerHeight) {
                Color alksPixel = GetStoredPixelColor(alks.pixels, alks.width, alks.height, x, y);
                Color playerPixel = GetStoredPixelColor(playerPixels, playerWidth, playerHeight, playerLocalPos.x, playerLocalPos.y);

                if (alksPixel.a > 0 && playerPixel.a > 0) {
                    return true;
                }
            }
        }
    }

    return false;
}

void mousePosition() {
    Vector2 mousePos;
    mousePos = GetMousePosition();

    if (IsKeyPressed(KEY_M)) {
        printf("\ny: %f", mousePos.y);
        printf("\nx: %f", mousePos.x);
        printf("\n\n\n\n");
    }
}

Vector2 GetRandomPosition() {
    Vector2 Position;
    Position.x = GetRandomValue(50, WW - 50);
    Position.y = GetRandomValue(50, WH - 50);
    return Position;
}

bool CheckPixelCollisionPlayerAndRuby() {
    Image playerImg = LoadImageFromTexture(playerSprite);
    Image rubyImg = LoadImageFromTexture(rubySprite);

    Color *playerPixels = LoadImageColors(playerImg);
    Color *rubyPixels = LoadImageColors(rubyImg);

    int pixelStartX = (player.Position.x > rubyPosition.x) ? player.Position.x : rubyPosition.x;
    int pixelStartY = (player.Position.y > rubyPosition.y) ? player.Position.y : rubyPosition.y;
    int pixelEndX = ((player.Position.x + playerSprite.width) < (rubyPosition.x + rubySprite.width)) ? \
                    (player.Position.x + playerSprite.width) : (rubyPosition.x + rubySprite.width);
    int pixelEndY = ((player.Position.y + playerSprite.height) < (rubyPosition.y + rubySprite.height)) ? \
                    (player.Position.y + playerSprite.height) : (rubyPosition.y + rubySprite.height);

    for (int y = pixelStartY; y < pixelEndY; y++) {
        for (int x = pixelStartX; x < pixelEndX; x++) {
            int playerImgX = x - player.Position.x;
            int playerImgY = y - player.Position.y;
            int rubyImgX = x - rubyPosition.x;
            int rubyImgY = y - rubyPosition.y;

            Color playerCol = playerPixels[playerImgY * playerSprite.width + playerImgX];
            Color rubyCol = rubyPixels[rubyImgY * rubySprite.width + rubyImgX];

            // for collision
            if (playerCol.a > 0 && rubyCol.a > 0) {
                UnloadImageColors(playerPixels);
                UnloadImageColors(rubyPixels);
                UnloadImage(playerImg);
                UnloadImage(rubyImg);
                return true;
            }
        }
    }

    // cleanup
    UnloadImageColors(playerPixels);
    UnloadImageColors(rubyPixels);
    UnloadImage(playerImg);
    UnloadImage(rubyImg);

    return false;
}

void respawnRuby() {
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barY = 10;
    int buffer = 20;

    do {
        rubyPosition = GetRandomPosition();
    } while ((rubyPosition.x >= barX - buffer && rubyPosition.x <= barX + barWidth + buffer) && 
            (rubyPosition.y >= barY - buffer && rubyPosition.y <= barY + barHeight + buffer));
}

void saveToFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        fputs(text, file);
        fclose(file);
    }
}

void loadFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        textLength = fread(text, 1, MAX_TEXT_SIZE - 1, file);
        text[textLength] = '\0';
        fclose(file);
    }
}

int main(void) {
    InitWindow(WW, WH, "Alks Revolution");
    SetExitKey(0);
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_WARNING);

    playerSprite = LoadTexture("res/mine/cube1.png");
    rubySprite = LoadTexture("res/mine/ruby.png");
    alksSprite = LoadTexture("res/mine/cube1.png");

    romulusFont = LoadFont("res/font/romulus.png");
    defaultFont = GetFontDefault();

    printf("\n\n\n\nProgram Initialized!\n\n\n\n\n");

    Image playerImage = LoadImageFromTexture(playerSprite);
    Color *playerPixels = LoadImageColors(playerImage);
    int playerWidth = playerSprite.width;
    int playerHeight = playerSprite.height;

    initAlks();
    respawnRuby();

    bool gameOver = false;

    bool isPaused = false;

    bool commandLine = false;

    double lastTime = 0;
    int show = 0;

    int cursorPos = 0;

    while (!WindowShouldClose()) {
        if (!gameOver && !isPaused && !commandLine) {
            for (int i = 0; i < ALKS_COUNT; i++) {
                float dx = player.Position.x - alks[i].Position.x;
                float dy = player.Position.y - alks[i].Position.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance > 0) {
                    alks[i].Position.x += (dx / distance) * alks[i].Speed;
                    alks[i].Position.y += (dy / distance) * alks[i].Speed;
                }

                if (CheckPixelCollisionPlayerAndEnemy(alks[i], playerPixels, playerWidth, playerHeight)) {
                    if (player.health > 0) {
                        player.health --;
                    }

                    respawnAlks(i);
                }

                if (player.health == 0) {
                    gameOver = true;
                }
            }

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.Position.y -= player.Dash;
                }

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.Position.y -= player.Run;
                }

                player.Position.y -= player.Speed;
            }

            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.Position.y += player.Dash;
                }

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.Position.y += player.Run;
                }

                player.Position.y += player.Speed;
            }

            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.Position.x -= player.Dash;
                }

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.Position.x -= player.Run;
                }

                player.Position.x -= player.Speed;
            }

            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.Position.x += player.Dash;
                }

                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.Position.x += player.Run;
                }

                player.Position.x += player.Speed;
            }

            if (IsKeyPressed(KEY_E)) {
                shootBullets();
            }

            updateBullets();
        }


        Rectangle topWall = { 0, -10, WW, 10 };
        Rectangle bottomWall = { 0, WH, WW, 10 };
        Rectangle leftWall = { -10, 0, 10, WH };
        Rectangle rightWall = { WW, 0, 10, WH };

        Rectangle playerRect = { player.Position.x, player.Position.y, playerSprite.width, playerSprite.height };

        if (CheckCollisionRecs(playerRect, topWall)) player.Position.y = 0;
        if (CheckCollisionRecs(playerRect, bottomWall)) player.Position.y = WH - playerSprite.height;
        if (CheckCollisionRecs(playerRect, leftWall)) player.Position.x  = 0;
        if (CheckCollisionRecs(playerRect, rightWall)) player.Position.x = WW - playerSprite.width;

        mousePosition();

        bool playerAndRubyCollision = CheckPixelCollisionPlayerAndRuby();

        if (playerAndRubyCollision) {
            respawnRuby();
        }

        if (IsKeyPressed(KEY_ESCAPE) && !commandLine && !gameOver) {
            isPaused = !isPaused;
        }

        if (IsKeyPressed(KEY_F2) && !isPaused && !gameOver) {
            commandLine = !commandLine;
        }

        if (commandLine) {
            int key = GetCharPressed();
            while (key > 0) {
                if (textLength < MAX_TEXT_SIZE - 1 && key >= 32 && key <= 125) {
                    for (int i = textLength; i > cursorPos; i--) {
                        text[i] = text[i - 1];
                    }
                    text[cursorPos] = (char)key;
                    textLength++;
                    cursorPos++;
                    text[textLength] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && cursorPos > 0) {
                for (int i = cursorPos - 1; i < textLength; i++) {
                    text[i] = text[i + 1];
                }
                textLength--;
                cursorPos--;
            }

            if (IsKeyPressed(KEY_RIGHT) && cursorPos < textLength) cursorPos++;
            if (IsKeyPressed(KEY_LEFT) && cursorPos > 0) cursorPos--;

            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) saveToFile("commandLine.txt");
            if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L)) loadFromFile("commandLine.txt");
        }


        float cursorX = 70;

        for (i = 0; i < cursorPos; i++) {
            cursorX += 19;
        }

        double currentTime = GetTime();

        if (currentTime - lastTime >= 1.0) {
            show = !show;
            lastTime = currentTime;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
            if (!gameOver) {
                drawPlayerHealthBar(&player);
                DrawTexture(rubySprite, rubyPosition.x, rubyPosition.y, WHITE);

                DrawTexture(playerSprite, player.Position.x, player.Position.y, WHITE);

                DrawTexture(rubySprite, rubyPosition.x, rubyPosition.y, WHITE);

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].Active) {
                        DrawCircleV(bullets[i].Position, 5, BLACK);
                    }
                }

                for (int i = 0; i < ALKS_COUNT; i++) {
                    if (alks[i].Position.x != -999) {
                        DrawTexture(alksSprite, alks[i].Position.x, alks[i].Position.y, WHITE);
                        drawAlksHealthBar(&alks[i]);
                    }
                }
            } else {
                DrawText("Game Over!", HALF_WW - 60, HALF_WH, 30, BLACK);

                if (IsKeyPressed(KEY_R)) {
                    gameOver = false;
                    player.Position.x = HALF_WW;
                    player.Position.y = HALF_WH;
                    player.health = PLAYER_HEALTH_COUNT;
                    initAlks();
                }
            }

            if (isPaused) {
                DrawRectangle(0, 0, WW, WH, transparentRaywhite);

                DrawText("Paused!", HALF_WW - 60, HALF_WH, 30, BLACK);
            }

            if (commandLine) {
                DrawRectangle(0, 0, WW, WH, transparentRaywhite);

                DrawText(">", 40, WH - 70, 30, BLACK);

                if (fmod(GetTime(), 1.0) < 0.5) {
                    DrawText("|", cursorX, WH - 70, 30, BLACK);
                }

                DrawTextEx(defaultFont, text, (Vector2) { 70, WH - 70 }, 30, 4, BLACK);
            }
        EndDrawing();
    }

    UnloadTexture(playerSprite);
    UnloadTexture(rubySprite);
    UnloadTexture(alksSprite);

    free(playerPixels);
    free(alks[0].pixels);

    UnloadFont(romulusFont);

    CloseWindow();

    printf("\n\n\n\nProgram Closed!\n\n\n\n\n");

    return 0;
}

