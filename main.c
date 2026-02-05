#include "raylib.h"
#include "raymath.h"
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

#define MAX_BULLETS 6
#define MAX_MAGIC 5
#define BULLET_SPEED 3.0f

#define PLAYER_HEALTH_COUNT 5
#define ALKS_HEALTH_COUNT 3

#define MAX_TEXT_SIZE 1024

typedef enum ActualWeapon {
    NONE,
    GUN,
    SWORD,
    WAND,
    LASER
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

typedef struct {
    Vector2 start;
    Vector2 end;
    bool active;
    float duration;
} Laser;

struct Player player = {
    .speed = 4,
    .dash = 75,
    .run = 7.5,
    .position = (Vector2) { HALF_WW, HALF_WH },
    .health = PLAYER_HEALTH_COUNT
};

ActualWeapon actualWeapon;
Weapon gun;
Weapon sword;
Weapon wand;
Weapon laserer;

Vector2 rubyPosition;

Bullet bullets[MAX_BULLETS];
Bullet magic[MAX_MAGIC];

Enemy alks[ALKS_COUNT];

Laser laser;

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

bool CheckCollisionBulletAndEnemy(Bullet bullet, Enemy *alks) {
    Rectangle bulletRect = { bullet.position.x, bullet.position.y, 5, 5 };
    Rectangle alksRect = { alks->position.x, alks->position.y, alks->width, alks->height };

    return CheckCollisionRecs(bulletRect, alksRect);
}

bool CheckCollisionPointLineLaserAndEnemy(Vector2 point, Vector2 start, Vector2 end, float threshold) {
    float d1 = Vector2Distance(start, point);
    float d2 = Vector2Distance(point, end);
    float lineLength = Vector2Distance(start, end);
    return (d1 + d2) >= (lineLength - threshold) && (d1 + d2) <= (lineLength + threshold);
}

Vector2 NormalizeVector2(Vector2 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y);
    if (length != 0) {
        v.x /= length;
        v.y /= length;
    }
    return v;
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
                    // bullets[i].active = false;
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

void updateMagic() {
    for (int i = 0; i < MAX_MAGIC; i++) {
        if (magic[i].active) {
            magic[i].position.x += magic[i].velocity.x;
            magic[i].position.y += magic[i].velocity.y;

            if (magic[i].position.x < 0 || magic[i].position.x > WW ||
                magic[i].position.y < 0 || magic[i].position.y > WH) {
                magic[i].active = false;
            }
        }

        for (int j = 0; j < ALKS_COUNT; j++) {
            if (alks[j].position.x != -999) {
                if (CheckCollisionBulletAndEnemy(magic[i], &alks[j])) {
                    // magic[i].active = false;
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

Vector2 ExtendLine(Vector2 start, Vector2 direction, float maxLength) {
    Vector2 end;
    end.x = start.x + direction.x * maxLength;
    end.y = start.y + direction.y * maxLength;
    return end;
}

void updateLaser() {
    for (int i = 0; i < ALKS_COUNT; i++) {
        if (alks[i].position.x != -999) {
            if (CheckCollisionPointLineLaserAndEnemy(alks[i].position, laser.start, laser.end, 10.0f)) {
                alks[i].health--;
                if (alks[i].health <= 0) {
                    respawnAlks(i);
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

            if (playerCol.a > 0 && rubyCol.a > 0) {
                UnloadImageColors(playerPixels);
                UnloadImageColors(rubyPixels);
                UnloadImage(playerImg);
                UnloadImage(rubyImg);
                return true;
            }
        }
    }

    UnloadImageColors(playerPixels);
    UnloadImageColors(rubyPixels);
    UnloadImage(playerImg);
    UnloadImage(rubyImg);

    return false;
}

void playerKeybindings() {
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
}

void weaponKeybindings() {
    if (IsKeyPressed(KEY_ONE)) {
        actualWeapon = NONE;
    } else if (IsKeyPressed(KEY_TWO)) {
        actualWeapon = GUN;
    } else if (IsKeyPressed(KEY_THREE)) {
        actualWeapon = SWORD;
    } else if (IsKeyPressed(KEY_FOUR)) {
        actualWeapon = WAND;
    } else if (IsKeyPressed(KEY_FIVE)) {
        actualWeapon = LASER;
    }

    if (actualWeapon == GUN) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].position = player.position;

                    Vector2 mousePos = GetMousePosition();
                    Vector2 direction = { mousePos.x - player.position.x, mousePos.y - player.position.y };
                    bullets[i].velocity = NormalizeVector2(direction);

                    bullets[i].velocity.x *= BULLET_SPEED;
                    bullets[i].velocity.y *= BULLET_SPEED;

                    bullets[i].active = true;
                    break;
                }
            }
        }
    }

    if (actualWeapon == WAND) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < MAX_MAGIC; i++) {
                if (!magic[i].active) {
                    magic[i].position = player.position;

                    Vector2 mousePos = GetMousePosition();
                    Vector2 direction = { mousePos.x - player.position.x, mousePos.y - player.position.y };
                    magic[i].velocity = NormalizeVector2(direction);

                    magic[i].velocity.x *= BULLET_SPEED;
                    magic[i].velocity.y *= BULLET_SPEED;

                    magic[i].active = true;
                }
                break;
            }
        }
    }

    if (actualWeapon == LASER) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            laser.active = true;
            laser.start = player.position;

            Vector2 mousePos = GetMousePosition();
            Vector2 direction = { mousePos.x - player.position.x, mousePos.y - player.position.y };
            direction = NormalizeVector2(direction);

            float maxLength = 1000;
            laser.end = ExtendLine(laser.start, direction, maxLength);
        } else {
            laser.active = false;
        }
    }
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

            playerKeybindings();

            updateBullets();
            updateMagic();

            weaponKeybindings();
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

        Weapon sword;
        sword.scale = 1;
        sword.position.x = player.position.x - playerSprite.width - 1;
        sword.position.y = player.position.y - playerSprite.height - 1;

        Weapon wand;
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

                for (int i = 0; i < ALKS_COUNT; i++) {
                    if (alks[i].position.x != -999) {
                        DrawTexture(alksSprite, alks[i].position.x, alks[i].position.y, WHITE);
                        drawAlksHealthBar(&alks[i]);
                    }
                }

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].active) {
                        DrawCircleV(bullets[i].position, 2, BLACK);
                    }
                }

                for (int i = 0; i < MAX_MAGIC; i++) {
                    if (magic[i].active) {
                        DrawCircleV(magic[i].position, 25, BLUE);
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

                case LASER:
                    if (laser.active) {
                        DrawLineEx(laser.start, laser.end, 3, RED);
                    }
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

