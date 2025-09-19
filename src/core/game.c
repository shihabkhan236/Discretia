#include "game.h"
#include "player.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../algorithms/algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// External declaration of gameFont
extern Font gameFont;

// Global game instance
GameData game = {0};

// Heart texture
static Texture2D heartTexture = {0};



// Function to calculate centered, contiguous box positions
void CalculateBoxPositions(Rectangle* platforms, int arraySize) {
    int totalWidth = arraySize * BOX_SIZE;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    for (int i = 0; i < arraySize; i++) {
        platforms[i] = (Rectangle){
            startX + i * BOX_SIZE,  // Contiguous positioning (no gaps)
            ARRAY_Y_POSITION,
            BOX_SIZE,
            BOX_SIZE
        };
    }
}

void InitGame(void) {
    // Initialize game state
    game.currentState = STATE_MAIN_MENU;
    game.previousState = STATE_MAIN_MENU;
    
    // Load heart texture
    if (FileExists("resources/heart.png")) {
        heartTexture = LoadTexture("resources/heart.png");
        TraceLog(LOG_INFO, "Heart texture loaded successfully");
    } else {
        TraceLog(LOG_WARNING, "heart.png not found, using fallback");
        // Create a simple red square as fallback
        Image heartImg = GenImageColor(32, 32, RED);
        heartTexture = LoadTextureFromImage(heartImg);
        UnloadImage(heartImg);
    }
    
    // Initialize game data
    game.selectedAlgorithm = ALGO_BUBBLE_SORT;
    game.selectedLevel = 0;
    game.hearts = MAX_HEARTS;
    game.score = 0;
    game.gameComplete = false;
    
    // Initialize array
    game.arraySize = 5;
    for (int i = 0; i < game.arraySize; i++) {
        game.array[i] = i + 1;
    }
    
    // Initialize player (same size as platforms)
    game.player = (Rectangle){100, 300, BOX_SIZE, BOX_SIZE};
    game.velocity = (Vector2){0, 0};
    game.isOnGround = false;
    game.playerNumber = 0;
    game.carrying = false;
    
    // Initialize platforms (will be positioned by CalculateBoxPositions)
    for (int i = 0; i < game.arraySize; i++) {
        game.platforms[i] = (Rectangle){0, 0, 64, 64};
    }
    CalculateBoxPositions(game.platforms, game.arraySize);
    
    game.selectedButton = 0;
    game.buttonPressed = false;
    
    printf("Game initialized successfully\n");
}

void UpdateGame(void) {
    switch (game.currentState) {
        case STATE_MAIN_MENU:
            // Handle main menu input
            if (IsKeyPressed(KEY_DOWN)) {
                game.selectedButton = (game.selectedButton + 1) % 2;
            }
            if (IsKeyPressed(KEY_UP)) {
                game.selectedButton = (game.selectedButton - 1 + 2) % 2;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (game.selectedButton == 0) {
                    ChangeState(STATE_ALGORITHM_SELECT);
                } else {
                    // Quit game - will be handled by main loop
                }
            }
            // No backspace navigation from main menu (it's the root)
            break;
            
        case STATE_ALGORITHM_SELECT:
            // Handle algorithm selection
            if (IsKeyPressed(KEY_DOWN)) {
                game.selectedButton = (game.selectedButton + 2) % MAX_ALGORITHMS;
            }
            if (IsKeyPressed(KEY_UP)) {
                game.selectedButton = (game.selectedButton - 2 + MAX_ALGORITHMS) % MAX_ALGORITHMS;
            }
            if (IsKeyPressed(KEY_LEFT)) {
                if (game.selectedButton % 2 == 1) game.selectedButton--;
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                if (game.selectedButton % 2 == 0 && game.selectedButton + 1 < MAX_ALGORITHMS) {
                    game.selectedButton++;
                }
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                game.selectedAlgorithm = (AlgorithmType)game.selectedButton;
                ChangeState(STATE_LEVEL_SELECT);
            }
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                ChangeState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_LEVEL_SELECT:
            // Handle level selection
            if (IsKeyPressed(KEY_LEFT)) {
                game.selectedButton = (game.selectedButton - 1 + (MAX_LEVELS + 1)) % (MAX_LEVELS + 1);
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                game.selectedButton = (game.selectedButton + 1) % (MAX_LEVELS + 1);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (game.selectedButton < MAX_LEVELS) {
                    game.selectedLevel = game.selectedButton;
                    ResetLevel();
                    ChangeState(STATE_GAMEPLAY);
                } else {
                    // Back button
                    ChangeState(STATE_ALGORITHM_SELECT);
                }
            }
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                ChangeState(STATE_ALGORITHM_SELECT);
            }
            break;
            
        case STATE_GAMEPLAY:
            // Update player movement and physics
            UpdatePlayerMovement(&game);
            
            // Handle gameplay - delegate to algorithm
            AlgorithmFunctions* algo = GetAlgorithm(game.selectedAlgorithm);
            if (algo && algo->update) {
                algo->update(&game);
            }
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                ChangeState(STATE_PAUSE);
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                ChangeState(STATE_LEVEL_SELECT);
            }
            if (IsKeyPressed(KEY_R)) {
                ResetLevel();
            }
            break;
            
        case STATE_LEVEL_COMPLETE:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                if (game.selectedLevel < MAX_LEVELS - 1) {
                    game.selectedLevel++;
                    ResetLevel();
                    ChangeState(STATE_GAMEPLAY);
                } else {
                    ChangeState(STATE_ALGORITHM_SELECT);
                }
            }
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                ChangeState(STATE_LEVEL_SELECT);
            }
            break;
            
        case STATE_GAME_OVER:
            if (IsKeyPressed(KEY_R)) {
                ResetLevel();
                ChangeState(STATE_GAMEPLAY);
            }
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                ChangeState(STATE_LEVEL_SELECT);
            }
            break;
            
        case STATE_PAUSE:
            if (IsKeyPressed(KEY_ESCAPE)) {
                ChangeState(STATE_GAMEPLAY);
            }
            break;
    }
}

void RenderGame(void) {
    switch (game.currentState) {
        case STATE_MAIN_MENU:
            // Draw title with custom font
            DrawTextEx(gameFont, "DISCRETIA", (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(gameFont, "DISCRETIA", FONT_SIZE_TITLE, 2).x/2, 150}, FONT_SIZE_TITLE, 2, UI_TEXT_PRIMARY);
            DrawTextEx(gameFont, "AN ALGO VISUALIZER PLATFORMER GAME", 
                     (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(gameFont, "AN ALGO VISUALIZER PLATFORMER GAME", FONT_SIZE_BODY, 1).x/2, 200}, 
                     FONT_SIZE_BODY, 1, UI_TEXT_PRIMARY);
            
            // Draw buttons
            Rectangle startBtn = {SCREEN_WIDTH/2 - BUTTON_WIDTH/2, 300, BUTTON_WIDTH, BUTTON_HEIGHT};
            Rectangle quitBtn = {SCREEN_WIDTH/2 - BUTTON_WIDTH/2, 380, BUTTON_WIDTH, BUTTON_HEIGHT};
            
            Color startColor = (game.selectedButton == 0) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
            Color quitColor = (game.selectedButton == 1) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
            
            DrawRectangleRec(startBtn, startColor);
            DrawRectangleLinesEx(startBtn, 2, UI_BORDER);
            DrawTextEx(gameFont, "START GAME", 
                      (Vector2){startBtn.x + (startBtn.width - MeasureTextEx(gameFont, "START GAME", FONT_SIZE_BUTTON, 1).x)/2, 
                               startBtn.y + (startBtn.height - FONT_SIZE_BUTTON)/2}, 
                      FONT_SIZE_BUTTON, 1, UI_TEXT_PRIMARY);
            
            DrawRectangleRec(quitBtn, quitColor);
            DrawRectangleLinesEx(quitBtn, 2, UI_BORDER);
            DrawTextEx(gameFont, "QUIT", 
                      (Vector2){quitBtn.x + (quitBtn.width - MeasureTextEx(gameFont, "QUIT", FONT_SIZE_BUTTON, 1).x)/2, 
                               quitBtn.y + (quitBtn.height - FONT_SIZE_BUTTON)/2}, 
                      FONT_SIZE_BUTTON, 1, UI_TEXT_PRIMARY);
            break;
            
        case STATE_ALGORITHM_SELECT:
            DrawTextEx(gameFont, "SELECT ALGO", 
                      (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(gameFont, "SELECT ALGO", FONT_SIZE_TITLE, 2).x/2, 100}, 
                      FONT_SIZE_TITLE, 2, UI_TEXT_PRIMARY);
            
            // Draw algorithm buttons in 2x3 grid
            const char* algoNames[] = {"Bubble sort", "Selection sort", "Insertion sort", "Merge sort", "Quick sort"};
            int startX = SCREEN_WIDTH/2 - BUTTON_WIDTH - 10;
            int startY = 200;
            
            for (int i = 0; i < MAX_ALGORITHMS; i++) {
                int col = i % 2;
                int row = i / 2;
                Rectangle btn = {
                    startX + col * (BUTTON_WIDTH + 20),
                    startY + row * (BUTTON_HEIGHT + 20),
                    BUTTON_WIDTH,
                    BUTTON_HEIGHT
                };
                
                Color btnColor = (game.selectedButton == i) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
                DrawRectangleRec(btn, btnColor);
                DrawRectangleLinesEx(btn, 2, UI_BORDER);
                // Convert to uppercase for better appearance with Mecha font
                char algoNameUpper[32];
                strncpy(algoNameUpper, algoNames[i], sizeof(algoNameUpper) - 1);
                algoNameUpper[sizeof(algoNameUpper) - 1] = '\0';
                for (size_t j = 0; j < strlen(algoNameUpper); j++) {
                    algoNameUpper[j] = toupper(algoNameUpper[j]);
                }
                

                int fontSize = FONT_SIZE_BUTTON;
                Vector2 textSize = MeasureTextEx(gameFont, algoNameUpper, fontSize, 1);

                while (textSize.x > btn.width - 10 && fontSize > 10) {
                    fontSize--;
                    textSize = MeasureTextEx(gameFont, algoNameUpper, fontSize, 1);
                }

                DrawTextEx(gameFont, algoNameUpper,
                        (Vector2){btn.x + (btn.width - textSize.x)/2,
                                    btn.y + (btn.height - fontSize)/2},
                        fontSize, 1, UI_TEXT_PRIMARY);
                // DrawTextEx(gameFont, algoNameUpper, 
                //           (Vector2){btn.x + (btn.width - MeasureTextEx(gameFont, algoNameUpper, FONT_SIZE_BUTTON, 1).x)/2, 
                //                    btn.y + (btn.height - FONT_SIZE_BUTTON)/2}, 
                //           FONT_SIZE_BUTTON, 1, UI_TEXT_PRIMARY);
            }
            break;
            
        case STATE_LEVEL_SELECT:
            DrawTextEx(gameFont, "SELECT LEVEL", 
                      (Vector2){SCREEN_WIDTH/2 - MeasureTextEx(gameFont, "SELECT LEVEL", FONT_SIZE_TITLE, 2).x/2, 100}, 
                      FONT_SIZE_TITLE, 2, UI_TEXT_PRIMARY);
            
            // Draw level buttons horizontally
            int levelStartX = SCREEN_WIDTH/2 - (MAX_LEVELS * 80 ) / 2;
            int levelY = 250;
            
            for (int i = 0; i <= MAX_LEVELS; i++) {
                Rectangle btn = {levelStartX + i * 70, levelY, 70, 70};
                Color btnColor = (game.selectedButton == i) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
                
                DrawRectangleRec(btn, btnColor);
                DrawRectangleLinesEx(btn, 2, UI_BORDER);
                
                if (i == 0) {
                    DrawTextEx(gameFont, "?", 
                              (Vector2){btn.x + (btn.width - MeasureTextEx(gameFont, "?", FONT_SIZE_BUTTON, 1).x)/2, 
                                       btn.y + (btn.height - FONT_SIZE_BUTTON)/2}, 
                              FONT_SIZE_BUTTON * 1.5, 1, UI_TEXT_PRIMARY);
                } else if (i <= MAX_LEVELS) {
                    char levelText[8];
                    sprintf(levelText, "%d", i);
                    DrawTextEx(gameFont, levelText, 
                              (Vector2){btn.x + (btn.width - MeasureTextEx(gameFont, levelText, FONT_SIZE_BUTTON, 1).x)/2, 
                                       btn.y + (btn.height - FONT_SIZE_BUTTON)/2}, 
                              FONT_SIZE_BUTTON * 1.5, 1, UI_TEXT_PRIMARY);
                }
            }
            break;
            
        case STATE_GAMEPLAY:
            // Draw level info
            char levelText[32];
            sprintf(levelText, "LEVEL %d", game.selectedLevel + 1);
            DrawTextEx(gameFont, levelText, (Vector2){20, 20}, FONT_SIZE_BODY * 2, 1, UI_TEXT_PRIMARY);
            
            // Draw hearts
            const int heartSize = 30;
            const int heartSpacing = 40;
            for (int i = 0; i < game.hearts; i++) {
                if (heartTexture.id > 0) {
                    DrawTexture(heartTexture, SCREEN_WIDTH - 130 + i * heartSpacing, 20, WHITE);
                } else {
                    // Fallback: draw red square if texture failed to load
                    DrawRectangle(SCREEN_WIDTH - 100 + i * heartSpacing, 20, heartSize, heartSize, RED);
                }
            }
            
            // Draw instructions
            // DrawTextEx(gameFont,"instructions: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor" , (Vector2){20, 60 * 1.25}, FONT_SIZE_SMALL * 2, 1, UI_TEXT_PRIMARY);

            // DrawText("incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation", 
            //         20, 80 * 1.25, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            // DrawText("ullamco laboris nisi ut aliquip ex ea commodo consequat.", 
            //         20, 100 * 1.25, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            
            // Draw platforms and array elements
            for (int i = 0; i < game.arraySize; i++) {
                DrawRectangleRec(game.platforms[i], UI_BUTTON_NORMAL);
                DrawRectangleLinesEx(game.platforms[i], 2, UI_BORDER); // border
                
                if (game.array[i] != 0) {
                    char numText[8];
                    sprintf(numText, "%d", game.array[i]);
                    DrawCenteredText(numText, 
                                   game.platforms[i].x + game.platforms[i].width/2, 
                                   game.platforms[i].y + game.platforms[i].height/2 , 
                                   FONT_SIZE_BUTTON * 1.5, UI_TEXT_PRIMARY);
                }
            }
            
            // Draw player
            RenderPlayer(&game);
            
            // Delegate additional rendering to algorithm
            AlgorithmFunctions* algo = GetAlgorithm(game.selectedAlgorithm);
            if (algo && algo->render) {
                algo->render(&game);
            }
            break;
            
        case STATE_LEVEL_COMPLETE:
            DrawCenteredText("Level Complete!", SCREEN_WIDTH/2, 200, FONT_SIZE_TITLE, GAME_SORTED);
            DrawCenteredText("Press ENTER to continue", SCREEN_WIDTH/2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
            break;
            
        case STATE_GAME_OVER:
            DrawCenteredText("Game Over", SCREEN_WIDTH/2, 200, FONT_SIZE_TITLE, GAME_HEART);
            DrawCenteredText("Press R to retry or ESC for menu", SCREEN_WIDTH/2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
            break;
            
        case STATE_PAUSE:
            DrawCenteredText("Paused", SCREEN_WIDTH/2, 200, FONT_SIZE_TITLE, UI_TEXT_PRIMARY);
            DrawCenteredText("Press ESC to resume", SCREEN_WIDTH/2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
            break;
    }
}

void CleanupGame(void) {
    // Clean up heart texture if it was loaded
    if (heartTexture.id > 0) {
        UnloadTexture(heartTexture);
        heartTexture = (Texture2D){0};
    }
    
    // Cleanup algorithm-specific data
    AlgorithmFunctions* algo = GetAlgorithm(game.selectedAlgorithm);
    if (algo && algo->cleanup) {
        algo->cleanup(&game);
    }
    
    printf("Game cleaned up successfully\n");
}

void ChangeState(GameState newState) {
    game.previousState = game.currentState;
    game.currentState = newState;
    game.selectedButton = 0; // Reset button selection
    
    printf("State changed from %d to %d\n", game.previousState, game.currentState);
}

void ResetLevel(void) {
    // Reset game state for current level
    game.hearts = MAX_HEARTS;
    game.score = 0;
    game.gameComplete = false;
    
    // Reset algorithm-specific data
    AlgorithmFunctions* algo = GetAlgorithm(game.selectedAlgorithm);
    if (algo && algo->resetLevel) {
        algo->resetLevel(&game, game.selectedLevel);
    }
    
    // Recalculate box positions after array size might have changed
    CalculateBoxPositions(game.platforms, game.arraySize);
    
    // Reset player position to first platform
    game.player.x = game.platforms[0].x + (game.platforms[0].width - game.player.width) / 2;
    game.player.y = game.platforms[0].y - game.player.height;
    game.velocity = (Vector2){0, 0};
    game.isOnGround = true;
    
    printf("Level %d reset\n", game.selectedLevel);
}