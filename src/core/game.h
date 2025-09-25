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

// Completion Animation Types
typedef enum
{
    COMPLETION_INACTIVE,
    COMPLETION_VERIFYING,
    COMPLETION_DELAYING,
    COMPLETION_RIPPLING,
    COMPLETION_FINISHED
} CompletionAnimationPhase;

// Animation timing configuration
typedef struct
{
    float verificationDelay;      // 0.2s - brief pause for verification
    float preAnimationDelay;      // 0.3s - pause before ripple starts
    float rippleElementDelay;     // 0.15s - time between each element
    float postAnimationDelay;     // 0.5s - pause after ripple completes
} AnimationTimingConfig;

// Completion animation state
typedef struct
{
    CompletionAnimationPhase phase;
    float animationStartTime;
    float phaseStartTime;
    int currentRippleIndex;
    bool userInputDisabled;
    bool* elementHighlighted;
    int arraySize;
    AnimationTimingConfig timing;
} CompletionAnimationState;

// Forward declarations
typedef struct GameData GameData;
typedef struct Algorithm Algorithm;

// Centralized algorithm stats structure
typedef struct {
    char primaryStat[128];      // Main algorithm progress (e.g., "Pass: 2 | Comparisons: 5 | Swaps: 3")
    char secondaryStat[128];    // Secondary info (e.g., "Comparing 4 and 2")
    char instructionText[256];  // Current instruction (e.g., "Press F to pick up a number")
    char goalText[128];         // Algorithm goal (e.g., "Goal: Sort numbers in ascending order")
    Color instructionColor;     // Color for instruction text
    bool hasInstruction;        // Whether instruction text should be displayed
} AlgorithmStats;

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

    // Timer system
    float levelTimeLimit;    // Time limit for current level in seconds
    float currentTime;       // Current elapsed time in seconds
    bool timerActive;        // Whether timer is running

    // Add camera
    Camera2D camera;
    int cameraMode; // For switching between camera types

    // Completion animation system
    CompletionAnimationState completionAnimation;
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

// Timer functions
void InitTimer(int level);
void UpdateTimer(void);
void RenderTimer(void);
float CalculateLevelTimeLimit(int level);
void FormatTime(float timeInSeconds, char* buffer, int bufferSize);

// Layout functions
void CalculateBoxPositions(Rectangle *platforms, int arraySize);
void CalculateQuickSortBoxPositions(Rectangle *platforms, int arraySize, GameData *game);

// find the platform player is on
int GetPlayerPlatform(GameData *game);

// Completion Animation Manager functions
void InitCompletionAnimation(void);
void StartCompletionAnimation(void);
void UpdateCompletionAnimation(void);
bool IsCompletionAnimationActive(void);

// RippleAnimationSystem functions
void UpdateRippleAnimation(void);
void RenderCompletionHighlights(void);
bool IsRippleAnimationComplete(void);
void ResetRippleAnimation(void);

// CompletionVerificationSystem functions
bool VerifyArrayCompleteSorted(int *array, int size);
bool VerifyNoActiveOperations(GameData *game);
bool IsReadyForCompletion(GameData *game);
bool VerifyAlgorithmSpecificCompletion(GameData *game);

#endif // GAME_H