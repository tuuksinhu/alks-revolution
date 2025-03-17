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

typedef enum ActualWeapon {
    NONE,
    GUN,
    SWORD,
    WAND
} ActualWeapon;

typedef struct {
    Vector2 position;
    Vector2 direction;
    Vector2 velocity;
    bool active;
} Bullet;

typedef struct {
    float speed;
    Vector2 position;
    Texture2D sprite;
    Color *pixels;
    int health;
    int width, height;
    bool alive;
} Enemy;

typedef struct Player {
    int speed;
    int dash;
    float run;
    Vector2 position;
    int health;
} Player;

typedef struct Weapon {
    int rotation;
    int scale;
    int movement;
    int width;
    int height;
    Vector2 position;
} Weapon;

struct Player player = {
    .speed = 4,
    .dash = 75,
    .run = 7.5,
    .position = (Vector2) { HALF_WW, HALF_WH },
    .health = PLAYER_HEALTH_COUNT
};

ActualWeapon actualWeapon;
Weapon sword;
Weapon wand;

Vector2 rubyPosition;

Bullet bullets[MAX_BULLETS];

Enemy alks[ALKS_COUNT];

Texture2D playerSprite;
Texture2D rubySprite;
Texture2D alksSprite;
Texture2D swordSprite;
Texture2D wandSprite;

Font romulusFont;
Font defaultFont;

Color transparentRaywhite = (Color){245, 245, 245, 128};

char text[MAX_TEXT_SIZE] = { 0 };
int textLength = 0;

void initAlks() {
    srand(time(NULL));
    Image alksImage = LoadImageFromTexture(alksSprite);
    Color *alksPixels = LoadImageColors(alksImage);

    for (int i = 0; i < ALKS_COUNT; i++) {
        alks[i].position.x = rand() % (WW - alksSprite.width);
        alks[i].position.y = rand() % WH;
        alks[i].speed = (rand() % 3) + 1.5f;
        alks[i].pixels = alksPixels;
        alks[i].width = alksSprite.width;
        alks[i].height = alksSprite.height;
        alks[i].health = ALKS_HEALTH_COUNT;
    }

    UnloadImage(alksImage);
}

void respawnAlks(int index) {
    alks[index].position.x = rand() % (WW - alksSprite.width);
    alks[index].position.y = rand() % WH;
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
    Rectangle bulletRect = { bullet.position.x, bullet.position.y, 5, 5 };
    Rectangle alksRect = { alks->position.x, alks->position.y, alks->width, alks->height };

    return CheckCollisionRecs(bulletRect, alksRect);
}

void initBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
}

void shootBullets() {
    Vector2 moveDir = GetPlayerDirection();
    if (moveDir.x != 0 || moveDir.y != 0) {
        lastDirection = moveDir;
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].position = player.position;
            bullets[i].velocity.x = lastDirection.x * 10;
            bullets[i].velocity.y = lastDirection.y * 10;
            bullets[i].active = true;
            break;
        }
    }
}

void updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].position.x += bullets[i].velocity.x;
            bullets[i].position.y += bullets[i].velocity.y;

            if (bullets[i].position.x < 0 || bullets[i].position.x > WW ||
                bullets[i].position.y < 0 || bullets[i].position.y > WH) {
                bullets[i].active = false;
            }
        }

        for (int j = 0; j < ALKS_COUNT; j++) {
            if (alks[j].position.x != -999) {
                if (CheckCollisionBulletAndEnemy(bullets[i], &alks[j])) {
                    bullets[i].active = false;
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
    Rectangle healthBar = { alks->position.x, alks->position.y - 10, 30, 5 };
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
            Vector2 alksWorldPos = { alks.position.x + x, alks.position.y + y};
            Vector2 playerLocalPos = { alksWorldPos.x - player.position.x, alksWorldPos.y - player.position.y };

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
    Vector2 position;
    position.x = GetRandomValue(50, WW - 50);
    position.y = GetRandomValue(50, WH - 50);
    return position;
}

bool CheckPixelCollisionPlayerAndRuby() {
    Image playerImg = LoadImageFromTexture(playerSprite);
    Image rubyImg = LoadImageFromTexture(rubySprite);

    Color *playerPixels = LoadImageColors(playerImg);
    Color *rubyPixels = LoadImageColors(rubyImg);

    int pixelStartX = (player.position.x > rubyPosition.x) ? player.position.x : rubyPosition.x;
    int pixelStartY = (player.position.y > rubyPosition.y) ? player.position.y : rubyPosition.y;
    int pixelEndX = ((player.position.x + playerSprite.width) < (rubyPosition.x + rubySprite.width)) ? \
                    (player.position.x + playerSprite.width) : (rubyPosition.x + rubySprite.width);
    int pixelEndY = ((player.position.y + playerSprite.height) < (rubyPosition.y + rubySprite.height)) ? \
                    (player.position.y + playerSprite.height) : (rubyPosition.y + rubySprite.height);

    for (int y = pixelStartY; y < pixelEndY; y++) {
        for (int x = pixelStartX; x < pixelEndX; x++) {
            int playerImgX = x - player.position.x;
            int playerImgY = y - player.position.y;
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
    swordSprite = LoadTexture("res/weapons/sword.png");
    wandSprite = LoadTexture("res/weapons/wand.png");

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
                float dx = player.position.x - alks[i].position.x;
                float dy = player.position.y - alks[i].position.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance > 0) {
                    alks[i].position.x += (dx / distance) * alks[i].speed;
                    alks[i].position.y += (dy / distance) * alks[i].speed;
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
                    player.position.y -= player.dash;
                } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.position.y -= player.run;
                }

                player.position.y -= player.speed;
            } else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.position.y += player.dash;
                } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.position.y += player.run;
                }

                player.position.y += player.speed;
            } else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.position.x -= player.dash;
                } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.position.x -= player.run;
                }

                player.position.x -= player.speed;
            } else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                if (IsKeyPressed(KEY_SPACE)) {
                    player.position.x += player.dash;
                } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    player.position.x += player.run;
                }

                player.position.x += player.speed;
            }

            if (IsKeyPressed(KEY_E)) {
                shootBullets();
            }

            updateBullets();

            if (IsKeyPressed(KEY_ONE)) {
                actualWeapon = NONE;
            } else if (IsKeyPressed(KEY_TWO)) {
                actualWeapon = GUN;
            } else if (IsKeyPressed(KEY_THREE)) {
                actualWeapon = SWORD;
            } else if (IsKeyPressed(KEY_FOUR)) {
                actualWeapon = WAND;
            }
        }


        Rectangle topWall = { 0, -10, WW, 10 };
        Rectangle bottomWall = { 0, WH, WW, 10 };
        Rectangle leftWall = { -10, 0, 10, WH };
        Rectangle rightWall = { WW, 0, 10, WH };

        Rectangle playerRect = { player.position.x, player.position.y, playerSprite.width, playerSprite.height };

        if (CheckCollisionRecs(playerRect, topWall)) player.position.y = 0;
        if (CheckCollisionRecs(playerRect, bottomWall)) player.position.y = WH - playerSprite.height;
        if (CheckCollisionRecs(playerRect, leftWall)) player.position.x  = 0;
        if (CheckCollisionRecs(playerRect, rightWall)) player.position.x = WW - playerSprite.width;

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

        for (int i = 0; i < cursorPos; i++) {
            cursorX += 19;
        }

        double currentTime = GetTime();

        if (currentTime - lastTime >= 1.0) {
            show = !show;
            lastTime = currentTime;
        }

        Weapon sword;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            sword.rotation = 4;
        } else {
            sword.rotation = 0;
        }

        sword.scale = 1;

        sword.position.x = player.position.x - playerSprite.width - 1;
        sword.position.y = player.position.y - playerSprite.height - 1;

        Weapon wand;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            wand.rotation = 4;
        } else {
            wand.rotation = 0;
        }

        wand.scale = 1;

        wand.position.x = player.position.x - playerSprite.width - 1;
        wand.position.y = player.position.y - playerSprite.height - 1;

        BeginDrawing();
        ClearBackground(RAYWHITE);
            if (!gameOver) {
                drawPlayerHealthBar(&player);
                DrawTexture(rubySprite, rubyPosition.x, rubyPosition.y, WHITE);

                DrawTexture(playerSprite, player.position.x, player.position.y, WHITE);

                DrawTexture(rubySprite, rubyPosition.x, rubyPosition.y, WHITE);

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].active) {
                        DrawCircleV(bullets[i].position, 5, BLACK);
                    }
                }

                for (int i = 0; i < ALKS_COUNT; i++) {
                    if (alks[i].position.x != -999) {
                        DrawTexture(alksSprite, alks[i].position.x, alks[i].position.y, WHITE);
                        drawAlksHealthBar(&alks[i]);
                    }
                }

                switch (actualWeapon) {
                case NONE:

                    break;

                case GUN:

                    break;

                case SWORD:
                    DrawTextureEx(swordSprite, sword.position, sword.rotation, sword.scale, WHITE);
                    break;

                case WAND:
                    DrawTextureEx(wandSprite, wand.position, wand.rotation, wand.scale, WHITE);
                    break;
                }
            } else {
                DrawText("Game Over!", HALF_WW - 60, HALF_WH, 30, BLACK);

                if (IsKeyPressed(KEY_R)) {
                    gameOver = false;
                    player.position.x = HALF_WW;
                    player.position.y = HALF_WH;
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

