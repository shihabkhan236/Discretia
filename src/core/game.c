#include "game.h"
#include "player.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../algorithms/algorithm.h"
#include "../algorithms/quick_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Using default font - no external font needed

// Global game instance
GameData game = {0};

// Heart texture
static Texture2D heartTexture = {0};

void UpdateGameCamera(GameData *game)
{
    static float evenOutSpeed = 700;
    static bool eveningOut = false;
    static float evenOutTarget;

    Vector2 playerCenter = {game->player.x + game->player.width / 2,
                            game->player.y + game->player.height / 2};
    float deltaTime = GetFrameTime();

    game->camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    game->camera.target.x = playerCenter.x; // Always follow player horizontally

    if (eveningOut)
    {
        // Currently smoothing camera to target Y position
        if (evenOutTarget > game->camera.target.y)
        {
            game->camera.target.y += evenOutSpeed * deltaTime;
            if (game->camera.target.y > evenOutTarget)
            {
                game->camera.target.y = evenOutTarget;
                eveningOut = false;
            }
        }
        else
        {
            game->camera.target.y -= evenOutSpeed * deltaTime;
            if (game->camera.target.y < evenOutTarget)
            {
                game->camera.target.y = evenOutTarget;
                eveningOut = false;
            }
        }
    }
    else
    {
        // Check if player has landed and Y position is different from camera
        if (game->isOnGround &&
            (game->velocity.y == 0) &&
            (playerCenter.y != game->camera.target.y))
        {
            eveningOut = true;
            evenOutTarget = playerCenter.y;
        }
    }

    // Zoom control
    game->camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
    if (game->camera.zoom > 3.0f)
        game->camera.zoom = 3.0f;
    else if (game->camera.zoom < 0.25f)
        game->camera.zoom = 0.25f;
}

// Function to calculate centered, contiguous box positions
void CalculateBoxPositions(Rectangle *platforms, int arraySize)
{
    int totalWidth = arraySize * BOX_SIZE;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;

    for (int i = 0; i < arraySize; i++)
    {
        platforms[i] = (Rectangle){
            startX + i * BOX_SIZE, // Contiguous positioning (no gaps)
            ARRAY_Y_POSITION,
            BOX_SIZE,
            BOX_SIZE};
    }
}

// Function to calculate box positions with gaps for Quick Sort partitions
void CalculateQuickSortBoxPositions(Rectangle *platforms, int arraySize, GameData *game)
{
    if (!game->algorithmData)
    {
        // Fallback to regular positioning if no algorithm data
        CalculateBoxPositions(platforms, arraySize);
        return;
    }

    QuickSortData *data = (QuickSortData *)game->algorithmData;
    const int GAP_SIZE = 40; // 40 pixels gap between partitions as specified in the doc

    // Calculate total width including gaps
    int totalBoxWidth = arraySize * BOX_SIZE;
    int totalGaps = 0;

    // Add gaps: one after left partition (i+1), one after pivot (keeping pivot with current subarray)
    if (data->partitioning && data->i >= data->low)
    {
        totalGaps += GAP_SIZE; // Gap after left partition (≤ pivot)
    }
    if (data->partitioning && data->pivotIndex < data->high)
    {
        totalGaps += GAP_SIZE; // Gap after pivot (keeping pivot with current subarray)
    }

    int totalWidth = totalBoxWidth + totalGaps;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    int currentX = startX;

    for (int idx = 0; idx < arraySize; idx++)
    {
        // Add gap after left partition (elements ≤ pivot)
        if (data->partitioning && idx == data->i + 1 && data->i >= data->low)
        {
            currentX += GAP_SIZE;
        }

        // Add gap after pivot element (keeping pivot with current subarray)
        if (data->partitioning && idx == data->pivotIndex + 1 && data->pivotIndex < data->high)
        {
            currentX += GAP_SIZE;
        }

        platforms[idx] = (Rectangle){
            currentX,
            ARRAY_Y_POSITION,
            BOX_SIZE,
            BOX_SIZE};

        currentX += BOX_SIZE;
    }
}

int GetPlayerPlatform(GameData *game)
{
    for (int i = 0; i < game->arraySize; i++)
    {
        Rectangle platform = game->platforms[i];

        // Enhanced collision detection for Quick Sort state
        if (game->selectedAlgorithm == ALGO_QUICK_SORT)
        {
            if (QuickSortIsCompletedPivot(game, i))
            {
                // Adjusted collision positioning: move detection down with completed pivots
                platform.y += BOX_SIZE + (BOX_SIZE / 2);
            }
            else if (QuickSortIsSwappingElement(game, i))
            {
                // Dual-height collision checking: check elevated position first
                Rectangle elevatedPlatform = platform;
                elevatedPlatform.y -= BOX_SIZE + (BOX_SIZE / 2);

                // Priority 1: Check elevated position collision
                if (game->player.y + game->player.height <= elevatedPlatform.y + GROUND_TOLERANCE &&
                    game->player.y + game->player.height + game->velocity.y >= elevatedPlatform.y &&
                    game->player.x + game->player.width > elevatedPlatform.x + 8 &&
                    game->player.x < elevatedPlatform.x + elevatedPlatform.width - 8)
                {
                    game->player.y = elevatedPlatform.y - game->player.height;
                    game->velocity.y = 0;
                    game->isOnGround = true;
                    return i;
                }

                // Priority 2: Check original position (allows walking under elevated boxes)
                // Continue to standard collision check below
            }
        }

        // Standard collision check (for normal platforms and original height of elevated platforms)
        if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
            game->player.y + game->player.height + game->velocity.y >= platform.y &&
            game->player.x + game->player.width > platform.x + 8 &&
            game->player.x < platform.x + platform.width - 8)
        {

            game->player.y = platform.y - game->player.height;
            game->velocity.y = 0;
            game->isOnGround = true;
            return i;
        }
    }
    return -1;
}

void InitGame(void)
{
    // Initialize game state
    game.currentState = STATE_MAIN_MENU;
    game.previousState = STATE_MAIN_MENU;

    // Load heart texture
    if (FileExists("resources/heart.png"))
    {
        heartTexture = LoadTexture("resources/heart.png");
        TraceLog(LOG_INFO, "Heart texture loaded successfully");
    }
    else
    {
        TraceLog(LOG_WARNING, "heart.png not found, using fallback");
        // Create a simple red square as fallback
        Image heartImg = GenImageColor(32, 32, RED);
        heartTexture = LoadTextureFromImage(heartImg);
        UnloadImage(heartImg);
    }

    // Initialize camera
    game.camera = (Camera2D){0};
    game.camera.target = (Vector2){game.player.x + game.player.width / 2,
                                   game.player.y + game.player.height / 2};
    game.camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    game.camera.rotation = 0.0f;
    game.camera.zoom = 1.0f;
    game.cameraMode = 0;

    // Initialize game data
    game.selectedAlgorithm = ALGO_BUBBLE_SORT;
    game.selectedLevel = 0;
    game.hearts = MAX_HEARTS;
    game.score = 0;
    game.gameComplete = false;

    // Initialize array
    game.arraySize = 5;
    for (int i = 0; i < game.arraySize; i++)
    {
        game.array[i] = i + 1;
    }

    // Initialize player (same size as platforms)
    game.player = (Rectangle){100, 300, BOX_SIZE, BOX_SIZE};
    game.velocity = (Vector2){0, 0};
    game.isOnGround = false;
    game.playerNumber = 0;
    game.carrying = false;

    // Initialize platforms (will be positioned by CalculateBoxPositions)
    for (int i = 0; i < game.arraySize; i++)
    {
        game.platforms[i] = (Rectangle){0, 0, 64, 64};
    }
    CalculateBoxPositions(game.platforms, game.arraySize);

    game.selectedButton = 0;
    game.buttonPressed = false;

    // Initialize timer
    game.levelTimeLimit = 0.0f;
    game.currentTime = 0.0f;
    game.timerActive = false;

    printf("Game initialized successfully\n");
}

void UpdateGame(void)
{
    // Update timer (runs during gameplay)
    UpdateTimer();
    
    // running all the time
    switch (game.currentState)
    {
    case STATE_MAIN_MENU:
        // Handle main menu input
        if (IsKeyPressed(KEY_DOWN))
        {
            game.selectedButton = (game.selectedButton + 1) % 2;
        }
        if (IsKeyPressed(KEY_UP))
        {
            game.selectedButton = (game.selectedButton - 1 + 2) % 2;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            if (game.selectedButton == 0)
            {
                ChangeState(STATE_ALGORITHM_SELECT);
            }
            else
            {
                // Quit game - will be handled by main loop
            }
        }
        // No backspace navigation from main menu (it's the root)
        break;

    case STATE_ALGORITHM_SELECT:
        // Handle algorithm selection
        if (IsKeyPressed(KEY_DOWN))
        {
            game.selectedButton = (game.selectedButton + 2) % MAX_ALGORITHMS;
        }
        if (IsKeyPressed(KEY_UP))
        {
            game.selectedButton = (game.selectedButton - 2 + MAX_ALGORITHMS) % MAX_ALGORITHMS;
        }
        if (IsKeyPressed(KEY_LEFT))
        {
            if (game.selectedButton % 2 == 1)
                game.selectedButton--;
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
            if (game.selectedButton % 2 == 0 && game.selectedButton + 1 < MAX_ALGORITHMS)
            {
                game.selectedButton++;
            }
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            game.selectedAlgorithm = (AlgorithmType)game.selectedButton;
            ChangeState(STATE_LEVEL_SELECT);
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
        {
            ChangeState(STATE_MAIN_MENU);
        }
        break;

    case STATE_LEVEL_SELECT:
        // Handle level selection
        if (IsKeyPressed(KEY_LEFT))
        {
            game.selectedButton = (game.selectedButton - 1 + (MAX_LEVELS + 1)) % (MAX_LEVELS + 1);
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
            game.selectedButton = (game.selectedButton + 1) % (MAX_LEVELS + 1);
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            if (game.selectedButton < MAX_LEVELS)
            {
                game.selectedLevel = game.selectedButton;
                ResetLevel();
                ChangeState(STATE_GAMEPLAY);
            }
            else
            {
                // Back button
                ChangeState(STATE_ALGORITHM_SELECT);
            }
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
        {
            ChangeState(STATE_ALGORITHM_SELECT);
        }
        break;

    case STATE_GAMEPLAY:
        // Update player movement and physics
        UpdatePlayerMovement(&game);

        // Add camera update
        UpdateGameCamera(&game);

        // Handle gameplay - delegate to algorithm
        AlgorithmFunctions *algo = GetAlgorithm(game.selectedAlgorithm);
        if (algo && algo->update)
        {

            algo->update(&game);
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            ChangeState(STATE_PAUSE);
        }
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            ChangeState(STATE_LEVEL_SELECT);
        }
        if (IsKeyPressed(KEY_R))
        {
            ResetLevel();
        }
        break;

    case STATE_LEVEL_COMPLETE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            if (game.selectedLevel < MAX_LEVELS - 1)
            {
                game.selectedLevel++;
                ResetLevel();
                ChangeState(STATE_GAMEPLAY);
            }
            else
            {
                ChangeState(STATE_ALGORITHM_SELECT);
            }
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
        {
            ChangeState(STATE_LEVEL_SELECT);
        }
        break;

    case STATE_GAME_OVER:
        if (IsKeyPressed(KEY_R))
        {
            ResetLevel();
            ChangeState(STATE_GAMEPLAY);
        }
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
        {
            ChangeState(STATE_LEVEL_SELECT);
        }
        break;

    case STATE_PAUSE:
        if (IsKeyPressed(KEY_ESCAPE))
        {
            ChangeState(STATE_GAMEPLAY);
        }
        break;
    }
}

void RenderGame(void)
{
    switch (game.currentState)
    {
    case STATE_MAIN_MENU:
        // Draw title with default font
        DrawCenteredText("DISCRETIA", SCREEN_WIDTH / 2, 150, FONT_SIZE_TITLE, UI_TEXT_PRIMARY);
        DrawCenteredText("AN ALGO VISUALIZER PLATFORMER GAME", SCREEN_WIDTH / 2, 200, FONT_SIZE_BODY, UI_TEXT_PRIMARY);

        // Draw buttons
        Rectangle startBtn = {SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, 300, BUTTON_WIDTH, BUTTON_HEIGHT};
        Rectangle quitBtn = {SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, 380, BUTTON_WIDTH, BUTTON_HEIGHT};

        Color startColor = (game.selectedButton == 0) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
        Color quitColor = (game.selectedButton == 1) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;

        DrawRectangleRec(startBtn, startColor);
        DrawRectangleLinesEx(startBtn, 2, UI_BORDER);
        DrawCenteredText("START GAME", startBtn.x + startBtn.width / 2, startBtn.y + startBtn.height / 2, FONT_SIZE_BUTTON, UI_TEXT_PRIMARY);

        DrawRectangleRec(quitBtn, quitColor);
        DrawRectangleLinesEx(quitBtn, 2, UI_BORDER);
        DrawCenteredText("QUIT", quitBtn.x + quitBtn.width / 2, quitBtn.y + quitBtn.height / 2, FONT_SIZE_BUTTON, UI_TEXT_PRIMARY);
        break;

    case STATE_ALGORITHM_SELECT:
        DrawCenteredText("SELECT ALGO", SCREEN_WIDTH / 2, 100, FONT_SIZE_TITLE, UI_TEXT_PRIMARY);

        // Draw algorithm buttons in 2x3 grid
        const char *algoNames[] = {"Bubble sort", "Selection sort", "Insertion sort", "Merge sort", "Quick sort"};
        int startX = SCREEN_WIDTH / 2 - BUTTON_WIDTH - 10;
        int startY = 200;

        for (int i = 0; i < MAX_ALGORITHMS; i++)
        {
            int col = i % 2;
            int row = i / 2;
            Rectangle btn = {
                startX + col * (BUTTON_WIDTH + 20),
                startY + row * (BUTTON_HEIGHT + 20),
                BUTTON_WIDTH,
                BUTTON_HEIGHT};

            Color btnColor = (game.selectedButton == i) ? UI_BUTTON_HOVER : UI_BUTTON_NORMAL;
            DrawRectangleRec(btn, btnColor);
            DrawRectangleLinesEx(btn, 2, UI_BORDER);
            // Convert to uppercase for better appearance with Mecha font
            char algoNameUpper[32];
            strncpy(algoNameUpper, algoNames[i], sizeof(algoNameUpper) - 1);
            algoNameUpper[sizeof(algoNameUpper) - 1] = '\0';
            for (size_t j = 0; j < strlen(algoNameUpper); j++)
            {
                algoNameUpper[j] = toupper(algoNameUpper[j]);
            }

            int fontSize = FONT_SIZE_BUTTON;
            int textWidth = MeasureText(algoNameUpper, fontSize);

            while (textWidth > btn.width - 10 && fontSize > 10)
            {
                fontSize--;
                textWidth = MeasureText(algoNameUpper, fontSize);
            }

            DrawCenteredText(algoNameUpper, btn.x + btn.width / 2, btn.y + btn.height / 2, fontSize, UI_TEXT_PRIMARY);
            // DrawTextEx(gameFont, algoNameUpper,
            //           (Vector2){btn.x + (btn.width - MeasureTextEx(gameFont, algoNameUpper, FONT_SIZE_BUTTON, 1).x)/2,
            //                    btn.y + (btn.height - FONT_SIZE_BUTTON)/2},
            //           FONT_SIZE_BUTTON, 1, UI_TEXT_PRIMARY);
        }
        break;

    case STATE_LEVEL_SELECT:
        DrawCenteredText("SELECT LEVEL", SCREEN_WIDTH / 2, 100, FONT_SIZE_TITLE, UI_TEXT_PRIMARY);

        // Draw level buttons horizontally
        int levelStartX = SCREEN_WIDTH / 2 - (MAX_LEVELS * 80) / 2;
        int levelY = 250;

        for (int i = 0; i < MAX_LEVELS; i++)
        {
            Rectangle btn = {levelStartX + i * 70, levelY, 70, 70};
            Color btnColor = (game.selectedButton == i) ? BLACK : LIGHTGRAY;

            DrawRectangleLinesEx(btn, 2, btnColor);

            if (i == 0)
            {
                DrawCenteredText("?", btn.x + btn.width / 2, btn.y + btn.height / 2, FONT_SIZE_BUTTON * 1.5, UI_TEXT_PRIMARY);
            }
            else if (i <= MAX_LEVELS)
            {
                char levelText[8];
                sprintf(levelText, "%d", i);
                DrawCenteredText(levelText, btn.x + btn.width / 2, btn.y + btn.height / 2, FONT_SIZE_BUTTON * 1.5, UI_TEXT_PRIMARY);
            }
        }
        break;

    case STATE_GAMEPLAY:
        // Draw level info
        char levelText[32];
        sprintf(levelText, "LEVEL %d", game.selectedLevel + 1);
        DrawText(levelText, 20, 20, FONT_SIZE_BODY * 2, UI_TEXT_PRIMARY);

        // Draw hearts
        const int heartSize = 30;
        const int heartSpacing = 40;
        for (int i = 0; i < game.hearts; i++)
        {
            if (heartTexture.id > 0)
            {
                DrawTexture(heartTexture, SCREEN_WIDTH - 130 + i * heartSpacing, 20, WHITE);
            }
            else
            {
                // Fallback: draw red square if texture failed to load
                DrawRectangle(SCREEN_WIDTH - 100 + i * heartSpacing, 20, heartSize, heartSize, RED);
            }
        }

        // Draw timer at top center
        RenderTimer();

        // Start camera mode for world objects
        BeginMode2D(game.camera);

        // Draw platforms
        for (int i = 0; i < game.arraySize; i++)
        {
            // Choose color and position based on Quick Sort state
            Color platformColor = UI_BUTTON_NORMAL;
            Color borderColor = LIGHTGRAY;
            Rectangle platformRect = game.platforms[i];
            bool isCompletedPivot = false;

            if (game.selectedAlgorithm == ALGO_QUICK_SORT)
            {
                if (QuickSortIsCompletedPivot(&game, i))
                {
                    isCompletedPivot = true;
                    // Keep normal background color for completed pivots
                    platformColor = UI_BUTTON_NORMAL;
                    // Move completed pivots 1.5 rectangles lower
                    platformRect.y += BOX_SIZE + (BOX_SIZE / 2);
                }
                else if (QuickSortIsSwappingElement(&game, i))
                {
                    // Elements involved in pivot swapping go 1.5 rectangles higher
                    platformRect.y -= BOX_SIZE + (BOX_SIZE / 2);
                }
            }

            // Get player platform for highlighting
            int playerPlatformIndex = GetPlayerPlatform(&game);

            // Draw the platform rectangle
            DrawRectangleRec(platformRect, platformColor);

            // Choose border color - green for completed pivots, otherwise normal logic
            if (isCompletedPivot)
            {
                borderColor = QUICK_COMPLETED; // Green border for completed pivots
            }
            else
            {
                borderColor = (playerPlatformIndex == i) ? BLACK : LIGHTGRAY;
            }
            DrawRectangleLinesEx(platformRect, 2, borderColor);

            // Draw the number on the platform (at the adjusted position)
            if (game.array[i] != 0)
            {
                char numText[8];
                sprintf(numText, "%d", game.array[i]);

                DrawCenteredText(numText,
                                 platformRect.x + platformRect.width / 2,
                                 platformRect.y + platformRect.height / 2,
                                 FONT_SIZE_BUTTON * 1.5, UI_TEXT_PRIMARY);
            }
        }

        // Draw player
        RenderPlayer(&game);

        // Delegate additional rendering to algorithm
        AlgorithmFunctions *algo = GetAlgorithm(game.selectedAlgorithm);
        if (algo && algo->render)
        {
            algo->render(&game);
        }

        // End camera mode
        EndMode2D();

        // Render algorithm stats outside camera view (fixed screen position)
        if (algo && algo->getStats)
        {
            AlgorithmStats stats = {0};
            algo->getStats(&game, &stats);

            // Render stats in fixed screen coordinates
            if (strlen(stats.primaryStat) > 0)
            {
                DrawText(stats.primaryStat, 20, 120 - 20, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            }
            if (strlen(stats.secondaryStat) > 0)
            {
                DrawText(stats.secondaryStat, 20, 140 - 20, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            }
            if (stats.hasInstruction && strlen(stats.instructionText) > 0)
            {
                DrawText(stats.instructionText, 20, 160 - 20, FONT_SIZE_SMALL, stats.instructionColor);
            }
            if (strlen(stats.goalText) > 0)
            {
                DrawText(stats.goalText, 20, 200 - 20, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            }
        }

        break;

    case STATE_LEVEL_COMPLETE:
        DrawCenteredText("Level Complete!", SCREEN_WIDTH / 2, 200, FONT_SIZE_TITLE, GAME_SORTED);
        DrawCenteredText("Press ENTER to continue", SCREEN_WIDTH / 2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
        break;

    case STATE_GAME_OVER:
        DrawCenteredText("Game Over", SCREEN_WIDTH / 2, 200, FONT_SIZE_TITLE, GAME_HEART);
        
        // Show reason for game over
        if (game.currentTime >= game.levelTimeLimit) {
            DrawCenteredText("Time's Up!", SCREEN_WIDTH / 2, 250, FONT_SIZE_BODY, RED);
        } else if (game.hearts <= 0) {
            DrawCenteredText("No Hearts Left!", SCREEN_WIDTH / 2, 250, FONT_SIZE_BODY, RED);
        }
        
        DrawCenteredText("Press R to retry or ESC for menu", SCREEN_WIDTH / 2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
        break;

    case STATE_PAUSE:
        DrawCenteredText("Paused", SCREEN_WIDTH / 2, 200, FONT_SIZE_TITLE, UI_TEXT_PRIMARY);
        DrawCenteredText("Press ESC to resume", SCREEN_WIDTH / 2, 300, FONT_SIZE_BODY, UI_TEXT_PRIMARY);
        break;
    }
}

void CleanupGame(void)
{
    // Cleanup algorithm-specific data
    AlgorithmFunctions *algo = GetAlgorithm(game.selectedAlgorithm);
    if (algo && algo->cleanup)
        algo->cleanup(&game);

    // Clean up heart texture if it was loaded
    if (heartTexture.id > 0)
    {
        UnloadTexture(heartTexture);
        heartTexture = (Texture2D){0};
    }

    printf("Game cleaned up successfully\n");
}

void ChangeState(GameState newState)
{
    game.previousState = game.currentState;
    game.currentState = newState;
    game.selectedButton = 0; // Reset button selection

    // Handle timer state changes
    if (newState == STATE_GAMEPLAY) {
        game.timerActive = true; // Resume timer when entering gameplay
    } else if (newState == STATE_PAUSE) {
        game.timerActive = false; // Pause timer when pausing game
    } else if (newState == STATE_LEVEL_COMPLETE || newState == STATE_GAME_OVER) {
        game.timerActive = false; // Stop timer when level ends
    } else {
        game.timerActive = false; // Stop timer for menu states
    }

    printf("State changed from %d to %d\n", game.previousState, game.currentState);
}

void ResetLevel(void)
{
    // Reset game state for current level
    game.hearts = MAX_HEARTS;
    game.score = 0;
    game.gameComplete = false;

    /* 1.  free previous algorithm memory (if any) ---------------------- */
    AlgorithmFunctions *algo = GetAlgorithm(game.selectedAlgorithm);
    if (algo && algo->cleanup)
        algo->cleanup(&game);

    /* 2.  allocate / initialise new algorithm data --------------------- */
    if (algo && algo->init)
        algo->init(&game);

    /* 3.  let the algorithm set up the level array --------------------- */
    if (algo && algo->resetLevel)
        algo->resetLevel(&game, game.selectedLevel);
    // Recalculate box positions after array size might have changed
    CalculateBoxPositions(game.platforms, game.arraySize);

    // Reset player position to first platform
    game.player.x = game.platforms[0].x + (game.platforms[0].width - game.player.width) / 2;
    game.player.y = game.platforms[0].y - game.player.height;
    game.velocity = (Vector2){0, 0};
    game.isOnGround = true;

    // Initialize timer for the level
    InitTimer(game.selectedLevel);

    printf("Level %d reset\n", game.selectedLevel);
}

// Timer Functions Implementation

float CalculateLevelTimeLimit(int level) {
    if (level == 0) {
        return 30.0f; // Level 1 (index 0): 60 seconds
    }
    return 30.0f + (15.0f * (level)); // Level 2+: 60 + (20 * level)
}

void InitTimer(int level) {
    game.levelTimeLimit = CalculateLevelTimeLimit(level);
    game.currentTime = 0.0f;
    game.timerActive = true;
    
    printf("Timer initialized for level %d: %.0f seconds\n", level + 1, game.levelTimeLimit);
}

void UpdateTimer(void) {
    if (!game.timerActive || game.currentState != STATE_GAMEPLAY) {
        return;
    }
    
    game.currentTime += GetFrameTime();
    
    // Check if time is up
    if (game.currentTime >= game.levelTimeLimit) {
        game.timerActive = false;
        printf("Time's up! Game Over.\n");
        ChangeState(STATE_GAME_OVER);
    }
}

void FormatTime(float timeInSeconds, char* buffer, int bufferSize) {
    int totalSeconds = (int)timeInSeconds;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    snprintf(buffer, bufferSize, "%02d:%02d", minutes, seconds);
}

void RenderTimer(void) {
    if (game.currentState != STATE_GAMEPLAY || !game.timerActive) {
        return;
    }
    
    float remainingTime = game.levelTimeLimit - game.currentTime;
    if (remainingTime < 0) remainingTime = 0;
    
    char timeBuffer[16];
    FormatTime(remainingTime, timeBuffer, sizeof(timeBuffer));
    
    // Calculate position for center of screen
    int textWidth = MeasureText(timeBuffer, FONT_SIZE_TITLE);
    int x = (SCREEN_WIDTH - textWidth) / 2;
    int y = 20; // Top of screen with some padding
    
    // Choose color based on remaining time
    Color timerColor = UI_TEXT_PRIMARY;
    if (remainingTime <= 10.0f) {
        timerColor = RED; // Red when less than 10 seconds
    } else if (remainingTime <= 30.0f) {
        timerColor = ORANGE; // Orange when less than 30 seconds
    }
    
    // Draw timer with background for better visibility
    // DrawRectangle(x - 10, y - 5, textWidth + 20, FONT_SIZE_TITLE + 10, (Color){0, 0, 0, 128});
    DrawText(timeBuffer, x, y, FONT_SIZE_TITLE, timerColor);
}