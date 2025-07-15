#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "raymath.h"

#define PLAYER_WIDTH 64
#define PLAYER_HEIGHT 64
#define GRAVITY 0.6f
#define JUMP_FORCE -12.0f
#define MOVE_SPEED 5.0f
#define MAX_HEARTS 3
#define MAX_LEVELS 5
#define NUM_FLOATING_PLATFORMS 5

// Game states
typedef enum { MENU, GAMEPLAY, LEVEL_COMPLETE, GAME_OVER, LEVEL_SELECT } GameState;

// Level data: array values and platform (square) positions
const int levelArrays[MAX_LEVELS][NUM_FLOATING_PLATFORMS] = {
    {7, 2, 9, 4, 6},
    {3, 8, 1, 5, 10},
    {12, 7, 15, 2, 9},
    {5, 1, 8, 3, 6},
    {10, 2, 8, 4, 12}
};
// Adjusted levelSquarePositions for all levels to fit within 900x600 window
const int levelSquarePositions[MAX_LEVELS][NUM_FLOATING_PLATFORMS * 2] = {
    // Level 1: mixed spacing (easy)
    {120, 400, 220, 400, 400, 400, 520, 400, 700, 400},
    // Level 2: spaced, all inside window
    {80, 420, 240, 350, 400, 420, 560, 350, 720, 420},
    // Level 3: easier vertical spread
    {100, 440, 260, 380, 420, 440, 580, 380, 740, 440},
    // Level 4: spaced, all inside window
    {120, 420, 260, 350, 400, 420, 540, 350, 680, 420},
    // Level 5: spaced, will move vertically
    {120, 400, 320, 320, 520, 400, 720, 320, 920, 400}
};
// For moving squares in harder levels
float squareOffsets[MAX_LEVELS][NUM_FLOATING_PLATFORMS] = {0};
float squareSpeeds[MAX_LEVELS][NUM_FLOATING_PLATFORMS] = {
    {0, 0, 0, 0, 0}, // Level 1 static
    {0, 0, 0, 0, 0}, // Level 2 static
    {0, 0, 0, 0, 0}, // Level 3 static
    {0.8f, 1.2f, 1.0f, 0.7f, 1.3f}, // Level 4 moving
    {1.1f, 0.9f, 1.3f, 0.8f, 1.2f}  // Level 5 moving
};

// Helper function to reset the game state
void resetGame(Rectangle *player, Vector2 *velocity, int *score, int *hearts, bool *gameOver, int *arr, int *sorted, bool *levelComplete, int level, Rectangle *squares, int *playerNumber, bool *carrying, int *lastOnSquare) {
    int initial[NUM_FLOATING_PLATFORMS];
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) initial[i] = levelArrays[level][i];
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) arr[i] = initial[i];
    *score = 0;
    *hearts = MAX_HEARTS;
    *gameOver = false;
    *levelComplete = false;
    *sorted = 0;
    *lastOnSquare = 0;
    int sq = 0;
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
        squares[i].x = levelSquarePositions[level][sq++];
        squares[i].y = levelSquarePositions[level][sq++];
        squares[i].width = 64;
        squares[i].height = 64;
    }
    // Player starts on top of the first square
    player->x = squares[0].x + (squares[0].width - PLAYER_WIDTH) / 2;
    player->y = squares[0].y - PLAYER_HEIGHT;
    *velocity = (Vector2){0, 0};
    *playerNumber = 0;
    *carrying = false;
}

// Check if array is sorted in descending order
bool isSortedDescending(int *arr) {
    for (int i = 0; i < NUM_FLOATING_PLATFORMS - 1; i++) {
        if (arr[i] < arr[i + 1]) return false;
    }
    return true;
}

// Check if array is sorted in descending order and all boxes are filled
bool isSortedDescendingFull(int *arr) {
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
        if (arr[i] == 0) return false;
    }
    for (int i = 0; i < NUM_FLOATING_PLATFORMS - 1; i++) {
        if (arr[i] < arr[i + 1]) return false;
    }
    return true;
}

// Draw a vertical gradient background
void DrawVerticalGradient(int width, int height, Color top, Color bottom) {
    for (int y = 0; y < height; y++) {
        float t = (float)y / (float)height;
        Color c = (Color){
            (unsigned char)(top.r * (1 - t) + bottom.r * t),
            (unsigned char)(top.g * (1 - t) + bottom.g * t),
            (unsigned char)(top.b * (1 - t) + bottom.b * t),
            255
        };
        DrawLine(0, y, width, y, c);
    }
}

// Draw a rounded rectangle with drop shadow
void DrawRoundedRectShadow(Rectangle rec, float roundness, int segments, Color color, Color shadow) {
    Rectangle shadowRec = rec;
    shadowRec.x += 4;
    shadowRec.y += 6;
    DrawRectangleRounded(shadowRec, roundness, segments, shadow);
    DrawRectangleRounded(rec, roundness, segments, color);
}

// Add color pulse for moving squares
Color getAnimatedSquareColor(int level, int i, float t, Color base) {
    if (level < 3) return base;
    float pulse = 0.15f * (sinf(t + i) + 1.0f); // 0.0 to 0.3
    return (Color){
        (unsigned char)fminf(255, base.r + pulse * 255),
        (unsigned char)fminf(255, base.g + pulse * 255),
        (unsigned char)fminf(255, base.b + pulse * 255),
        255
    };
}

int main(void) {
    InitWindow(900, 600, "Algo Run");
    SetTargetFPS(60);
    InitAudioDevice();

    // Load assets with error checking
    Sound levelCompleteSound = {0};
    Texture2D heartTexture = {0};
    
    // Try to load sound, create fallback if failed
    if (FileExists("level-completion.wav")) {
        levelCompleteSound = LoadSound("level-completion.wav");
    } else {
        printf("Warning: level-completion.wav not found\n");
    }
    
    // Try to load heart texture, create fallback if failed
    if (FileExists("heart.png")) {
        heartTexture = LoadTexture("heart.png");
    } else {
        printf("Warning: heart.png not found, using fallback\n");
        // Create a simple heart texture programmatically
        Image heartImg = GenImageColor(32, 32, RED);
        heartTexture = LoadTextureFromImage(heartImg);
        UnloadImage(heartImg);
    }

    // Colors
    Color gradTop = (Color){40, 40, 60, 255};
    Color gradBottom = (Color){10, 10, 20, 255};
    Color squareColor = (Color){80, 80, 120, 255};
    Color squareShadow = (Color){30, 30, 50, 120};
    Color playerColor = (Color){200, 220, 255, 255};
    Color playerShadow = (Color){80, 100, 140, 120};
    Color heartColor = RED;
    Color floatingTextColor = WHITE;
    Color arrayBorderColor = (Color){180, 180, 255, 255};
    Color highlightColor = (Color){255, 200, 0, 255};
    Color menuBtnColor = (Color){60, 60, 100, 255};
    Color menuBtnShadow = (Color){20, 20, 40, 120};
    Color menuBtnText = WHITE;
    Color completeColor = (Color){0, 220, 120, 255};

    // Game state
    GameState state = MENU;
    int currentLevel = 0;

    // Player
    Rectangle player = {0, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
    Vector2 velocity = {0, 0};
    bool isOnGround = false;
    int playerNumber = 0; // 0 means empty
    bool carrying = false;
    int lastOnSquare = 0;

    // Array squares (platforms)
    Rectangle squares[NUM_FLOATING_PLATFORMS];
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
        squares[i].x = levelSquarePositions[0][i * 2];
        squares[i].y = levelSquarePositions[0][i * 2 + 1];
        squares[i].width = 64;
        squares[i].height = 64;
    }

    // Array values (unsorted at start)
    int arr[NUM_FLOATING_PLATFORMS];
    for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) arr[i] = levelArrays[0][i];

    int score = 0;
    int hearts = MAX_HEARTS;
    bool gameOver = false;
    bool levelComplete = false;
    int sorted = 0;

    // Menu selection
    int menuSelected = 0;
    int menuCount = 3;
    const char *menuItems[] = {"Start Game", "Select Level", "Quit"};
    int selectLevelIndex = 0;

    // For swap logic
    int lastSwapIndex = -1;
    bool lastSwapInAir = false;

    resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, 0, squares, &playerNumber, &carrying, &lastOnSquare);

    while (!WindowShouldClose()) {
        // --- MENU ---
        if (state == MENU) {
            if (IsKeyPressed(KEY_DOWN)) menuSelected = (menuSelected + 1) % menuCount;
            if (IsKeyPressed(KEY_UP)) menuSelected = (menuSelected - 1 + menuCount) % menuCount;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (menuSelected == 0) {
                    currentLevel = 0;
                    resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, currentLevel, squares, &playerNumber, &carrying, &lastOnSquare);
                    state = GAMEPLAY;
                } else if (menuSelected == 1) {
                    selectLevelIndex = 0;
                    state = LEVEL_SELECT;
                } else if (menuSelected == 2) {
                    break;
                }
            }
        }
        // --- LEVEL SELECT ---
        else if (state == LEVEL_SELECT) {
            if (IsKeyPressed(KEY_DOWN)) selectLevelIndex = (selectLevelIndex + 1) % (MAX_LEVELS + 1);
            if (IsKeyPressed(KEY_UP)) selectLevelIndex = (selectLevelIndex - 1 + (MAX_LEVELS + 1)) % (MAX_LEVELS + 1);
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (selectLevelIndex < MAX_LEVELS) {
                    currentLevel = selectLevelIndex;
                    resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, currentLevel, squares, &playerNumber, &carrying, &lastOnSquare);
                    state = GAMEPLAY;
                } else {
                    state = MENU;
                }
            }
            if (IsKeyPressed(KEY_ESCAPE)) state = MENU;
        }
        // --- GAMEPLAY ---
        else if (state == GAMEPLAY) {
            if (IsKeyPressed(KEY_R)) {
                resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, currentLevel, squares, &playerNumber, &carrying, &lastOnSquare);
            }
            if (!gameOver && !levelComplete) {
                // Horizontal movement
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    player.x -= MOVE_SPEED;
                }
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    player.x += MOVE_SPEED;
                }

                // Gravity
                velocity.y += GRAVITY;
                player.y += velocity.y;

                // Square collision (platforms)
                isOnGround = false;
                int onSquare = -1;
                for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                    Rectangle sq = squares[i];
                    if (player.y + player.height <= sq.y + 10 &&
                        player.y + player.height + velocity.y >= sq.y &&
                        player.x + player.width > sq.x + 8 && player.x < sq.x + sq.width - 8) {
                        player.y = sq.y - player.height;
                        velocity.y = 0;
                        isOnGround = true;
                        onSquare = i;
                    }
                }
                if (onSquare != -1) lastOnSquare = onSquare;

                // Jump
                if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && isOnGround) {
                    velocity.y = JUMP_FORCE;
                    isOnGround = false;
                }

                // Find the empty box
                int emptyIndex = -1;
                for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                    if (arr[i] == 0) { emptyIndex = i; break; }
                }

                // Pick up or place number
                if (onSquare != -1 && IsKeyPressed(KEY_F)) {
                    if (!carrying && arr[onSquare] != 0) {
                        // Pick up number from box
                        playerNumber = arr[onSquare];
                        arr[onSquare] = 0;
                        carrying = true;
                    } else if (carrying && arr[onSquare] == 0) {
                        // Place number into empty box
                        arr[onSquare] = playerNumber;
                        playerNumber = 0;
                        carrying = false;
                    }
                }

                // Swap with adjacent
                if (carrying && onSquare != -1 && arr[onSquare] != 0 && emptyIndex != -1 && (onSquare == emptyIndex - 1 || onSquare == emptyIndex + 1) && IsKeyPressed(KEY_F)) {
                    // Check if swap is correct (move towards descending order)
                    int temp = arr[onSquare];
                    bool correct = false;
                    if (emptyIndex > onSquare) {
                        // Swapping right
                        if (playerNumber > arr[onSquare]) correct = true;
                    } else if (emptyIndex < onSquare) {
                        // Swapping left
                        if (playerNumber < arr[onSquare]) correct = true;
                    }
                    if (correct) {
                        arr[onSquare] = playerNumber;
                        playerNumber = temp;
                    } else {
                        // Wrong swap
                        hearts--;
                        if (hearts <= 0) {
                            gameOver = true;
                            state = GAME_OVER;
                        }
                    }
                }

                // Check for level complete - FIXED: Now transitions to LEVEL_COMPLETE state
                if (isSortedDescendingFull(arr)) {
                    levelComplete = true;
                    state = LEVEL_COMPLETE;
                    // Play sound if it was loaded successfully
                    if (levelCompleteSound.frameCount > 0) {
                        PlaySound(levelCompleteSound);
                    }
                }

                // Fall off screen
                if (player.y > GetScreenHeight()) {
                    hearts--;
                    if (hearts > 0) {
                        // Respawn on first square, keep player's box as is
                        player.x = squares[0].x + (squares[0].width - PLAYER_WIDTH) / 2;
                        player.y = squares[0].y - PLAYER_HEIGHT;
                        velocity = (Vector2){0, 0};
                    } else {
                        gameOver = true;
                        state = GAME_OVER;
                    }
                }
            }
            // Animate moving squares for harder levels
            if (currentLevel >= 3) {
                for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                    float baseY = levelSquarePositions[currentLevel][i * 2 + 1];
                    float speed = squareSpeeds[currentLevel][i];
                    squareOffsets[currentLevel][i] += speed * GetFrameTime();
                    // Smoother, slower oscillation
                    squares[i].y = baseY + 32 * sinf(squareOffsets[currentLevel][i] * 0.7f + i);
                }
            }
        }
        // --- LEVEL COMPLETE ---
        else if (state == LEVEL_COMPLETE) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (currentLevel < MAX_LEVELS - 1) {
                    currentLevel++;
                    resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, currentLevel, squares, &playerNumber, &carrying, &lastOnSquare);
                    state = GAMEPLAY;
                } else {
                    state = MENU;
                }
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = MENU;
            }
        }
        // --- GAME OVER ---
        else if (state == GAME_OVER) {
            if (IsKeyPressed(KEY_R)) {
                resetGame(&player, &velocity, &score, &hearts, &gameOver, arr, &sorted, &levelComplete, currentLevel, squares, &playerNumber, &carrying, &lastOnSquare);
                state = GAMEPLAY;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                state = MENU;
            }
        }

        // --- Draw ---
        BeginDrawing();
        ClearBackground(BLACK);
        DrawVerticalGradient(GetScreenWidth(), GetScreenHeight(), gradTop, gradBottom);

        if (state == MENU) {
            DrawText("ARRAY BUBBLE SORT PLATFORMER", 120, 80, 40, WHITE);
            int menuY = 220;
            for (int i = 0; i < menuCount; i++) {
                Rectangle btn = {340, menuY + i * 80, 220, 60};
                DrawRoundedRectShadow(btn, 0.4f, 16, menuBtnColor, menuBtnShadow);
                if (i == menuSelected) DrawRectangleRoundedLinesEx(btn, 0.4f, 16, 4, highlightColor);
                int tw = MeasureText(menuItems[i], 28);
                DrawText(menuItems[i], btn.x + (btn.width - tw) / 2, btn.y + 16, 28, menuBtnText);
            }
            DrawText("Use UP/DOWN and ENTER", 340, menuY + menuCount * 80 + 20, 20, LIGHTGRAY);
        }
        else if (state == LEVEL_SELECT) {
            DrawText("SELECT LEVEL", 320, 80, 40, WHITE);
            int baseY = 180;
            int boxW = 320, boxH = 50, boxX = (900 - boxW) / 2;
            for (int i = 0; i < MAX_LEVELS; i++) {
                Rectangle box = {boxX, baseY + i * (boxH + 16), boxW, boxH};
                DrawRoundedRectShadow(box, 0.3f, 12, menuBtnColor, menuBtnShadow);
                if (i == selectLevelIndex) DrawRectangleRoundedLinesEx(box, 0.3f, 12, 4, highlightColor);
                char levelText[32];
                snprintf(levelText, sizeof(levelText), "Level %d", i + 1);
                int tw = MeasureText(levelText, 28);
                DrawText(levelText, box.x + (box.width - tw) / 2, box.y + 12, 28, menuBtnText);
            }
            // Back button
            Rectangle backBox = {boxX, baseY + MAX_LEVELS * (boxH + 16), boxW, boxH};
            DrawRoundedRectShadow(backBox, 0.3f, 12, menuBtnColor, menuBtnShadow);
            if (selectLevelIndex == MAX_LEVELS) DrawRectangleRoundedLinesEx(backBox, 0.3f, 12, 4, highlightColor);
            int tw = MeasureText("Back", 28);
            DrawText("Back", backBox.x + (backBox.width - tw) / 2, backBox.y + 12, 28, menuBtnText);
            DrawText("UP/DOWN to select, ENTER to play, ESC to cancel", 200, backBox.y + boxH + 24, 20, LIGHTGRAY);
        }
        else if (state == GAMEPLAY) {
            // Draw array squares (platforms)
            for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                Color sqColor = getAnimatedSquareColor(currentLevel, i, squareOffsets[currentLevel][i], squareColor);
                DrawRoundedRectShadow(squares[i], 0.2f, 8, sqColor, squareShadow);
                DrawRectangleRoundedLinesEx(squares[i], 0.2f, 8, 3, arrayBorderColor);
                char numText[8];
                if (arr[i] != 0) snprintf(numText, sizeof(numText), "%d", arr[i]);
                else snprintf(numText, sizeof(numText), "");
                int textWidth = MeasureText(numText, 28);
                int textX = squares[i].x + (squares[i].width - textWidth) / 2;
                int textY = squares[i].y + (squares[i].height - 28) / 2;
                DrawText(numText, textX, textY, 28, floatingTextColor);
            }
            // Highlight swap area if player is between two squares and in the air
            for (int i = 0; i < NUM_FLOATING_PLATFORMS - 1; i++) {
                Rectangle sqA = squares[i];
                Rectangle sqB = squares[i+1];
                if (player.x + player.width/2 > sqA.x + sqA.width - 8 && player.x + player.width/2 < sqB.x + 8 &&
                    player.y + player.height > sqA.y - 40 && player.y + player.height < sqA.y + sqA.height + 40 && !isOnGround) {
                    Rectangle highlight = {
                        sqA.x + sqA.width - 8,
                        fminf(sqA.y, sqB.y) - 8,
                        (sqB.x + 8) - (sqA.x + sqA.width - 8),
                        80
                    };
                    DrawRectangleRoundedLinesEx(highlight, 0.2f, 8, 4, highlightColor);
                }
            }
            // Player shadow
            Rectangle playerShadowRec = player;
            playerShadowRec.x += 4;
            playerShadowRec.y += 8;
            DrawRectangleRounded(playerShadowRec, 0.2f, 8, playerShadow);
            // Player (as a square)
            DrawRectangleRounded(player, 0.2f, 8, playerColor);
            // Draw player's number if carrying
            if (carrying) {
                char numText[8];
                snprintf(numText, sizeof(numText), "%d", playerNumber);
                int textWidth = MeasureText(numText, 28);
                int textX = player.x + (player.width - textWidth) / 2;
                int textY = player.y + (player.height - 28) / 2;
                DrawText(numText, textX, textY, 28, BLACK);
            }

            // Score
            DrawText(TextFormat("Score: %d", score), 20, 20, 26, LIGHTGRAY);

            // Hearts - FIXED: Better heart rendering with fallback
            for (int i = 0; i < hearts; i++) {
                if (heartTexture.id > 0) {
                    DrawTexture(heartTexture, 860 - i * 40, 20, WHITE);
                } else {
                    // Fallback: draw heart as red rectangles
                    DrawRectangle(860 - i * 40, 20, 32, 32, RED);
                    DrawText("♥", 860 - i * 40 + 8, 20 + 4, 24, WHITE);
                }
            }

            // Instructions
            DrawText("Jump from square to square. Press F while jumping between squares to swap.", 80, 80, 20, LIGHTGRAY);
            DrawText("Sort the array in descending order!", 260, 110, 22, LIGHTGRAY);
            DrawText(TextFormat("Level: %d", currentLevel + 1), 20, 50, 20, LIGHTGRAY);
        }
        else if (state == LEVEL_COMPLETE) {
            // Draw the game background first
            // Draw array squares (platforms)
            for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                Color sqColor = getAnimatedSquareColor(currentLevel, i, squareOffsets[currentLevel][i], squareColor);
                DrawRoundedRectShadow(squares[i], 0.2f, 8, sqColor, squareShadow);
                DrawRectangleRoundedLinesEx(squares[i], 0.2f, 8, 3, arrayBorderColor);
                char numText[8];
                if (arr[i] != 0) snprintf(numText, sizeof(numText), "%d", arr[i]);
                else snprintf(numText, sizeof(numText), "");
                int textWidth = MeasureText(numText, 28);
                int textX = squares[i].x + (squares[i].width - textWidth) / 2;
                int textY = squares[i].y + (squares[i].height - 28) / 2;
                DrawText(numText, textX, textY, 28, floatingTextColor);
            }
            // Player
            DrawRectangleRounded(player, 0.2f, 8, playerColor);
            
            // Level complete overlay
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});
            DrawText("LEVEL COMPLETE!", 270, 200, 44, completeColor);
            if (currentLevel < MAX_LEVELS - 1) {
                DrawText("Press ENTER to go to next level", 220, 280, 28, LIGHTGRAY);
            } else {
                DrawText("All levels complete! Press ENTER for menu", 180, 280, 28, LIGHTGRAY);
            }
            DrawText("Press ESC to return to menu", 280, 320, 24, LIGHTGRAY);
        }
        else if (state == GAME_OVER) {
            // Draw the game background first
            // Draw array squares (platforms)
            for (int i = 0; i < NUM_FLOATING_PLATFORMS; i++) {
                Color sqColor = getAnimatedSquareColor(currentLevel, i, squareOffsets[currentLevel][i], squareColor);
                DrawRoundedRectShadow(squares[i], 0.2f, 8, sqColor, squareShadow);
                DrawRectangleRoundedLinesEx(squares[i], 0.2f, 8, 3, arrayBorderColor);
                char numText[8];
                if (arr[i] != 0) snprintf(numText, sizeof(numText), "%d", arr[i]);
                else snprintf(numText, sizeof(numText), "");
                int textWidth = MeasureText(numText, 28);
                int textX = squares[i].x + (squares[i].width - textWidth) / 2;
                int textY = squares[i].y + (squares[i].height - 28) / 2;
                DrawText(numText, textX, textY, 28, floatingTextColor);
            }
            // Player
            DrawRectangleRounded(player, 0.2f, 8, playerColor);
            
            // Game over overlay
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 128});
            DrawText("GAME OVER", 340, 200, 44, RED);
            DrawText("Press R to restart or ESC for menu", 220, 280, 28, LIGHTGRAY);
        }
        
        EndDrawing();
    }
    
    // Clean up resources
    if (levelCompleteSound.frameCount > 0) {
        UnloadSound(levelCompleteSound);
    }
    if (heartTexture.id > 0) {
        UnloadTexture(heartTexture);
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}