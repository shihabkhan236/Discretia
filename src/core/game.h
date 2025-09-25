#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

// Screen dimensions
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600

// Game constants
#define MAX_ALGORITHMS 5
#define MAX_LEVELS 5
#define MAX_ARRAY_SIZE 20
#define MAX_HEARTS 3

#define BOX_SIZE 64
#define BOX_BORDER_WIDTH 2
#define ARRAY_Y_POSITION 350

// Game States
typedef enum
{
    STATE_MAIN_MENU,
    STATE_ALGORITHM_SELECT,
    STATE_LEVEL_SELECT,
    STATE_GAMEPLAY,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_PAUSE
} GameState;

// Algorithm Types
typedef enum
{
    ALGO_BUBBLE_SORT,
    ALGO_SELECTION_SORT,
    ALGO_INSERTION_SORT,
    ALGO_MERGE_SORT,
    ALGO_QUICK_SORT
} AlgorithmType;

// Forward declarations
typedef struct GameData GameData;
typedef struct Algorithm Algorithm;

// Algorithm Interface
struct Algorithm
{
    char name[32];
    char description[256];
    int maxElements;
    int maxLevels;
    void (*initLevel)(int level, int *array, int size);
    bool (*processInput)(int input, GameData *game);
    void (*render)(GameData *game);
    bool (*isComplete)(int *array, int size);
};

// Game Data Structure
struct GameData
{
    GameState currentState;
    GameState previousState;

    // Current selections
    AlgorithmType selectedAlgorithm;
    int selectedLevel;

    // Game state
    int hearts;
    int score;
    bool gameComplete;

    // Array data
    int array[MAX_ARRAY_SIZE];
    int arraySize;

    // Player data
    Rectangle player;
    Vector2 velocity;
    bool isOnGround;
    int playerNumber;
    bool carrying;

    // Platform data
    Rectangle platforms[MAX_ARRAY_SIZE];

    // Algorithm-specific data
    void *algorithmData;

    // UI state
    int selectedButton;
    bool buttonPressed;

    // Add camera
    Camera2D camera;
    int cameraMode; // For switching between camera types
};

// Global game instance
extern GameData game;

// camera
void UpdateGameCamera(GameData *game);

// Core functions
void InitGame(void);
void UpdateGame(void);
void RenderGame(void);
void CleanupGame(void);

// State management
void ChangeState(GameState newState);
void ResetLevel(void);

// Layout functions
void CalculateBoxPositions(Rectangle *platforms, int arraySize);
void CalculateQuickSortBoxPositions(Rectangle *platforms, int arraySize, GameData *game);

// find the platform player is on
int GetPlayerPlatform(GameData *game);

#endif // GAME_H