#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include "../core/game.h"

// Quick Sort specific data structure for Lomuto partition
typedef struct
{
    int low;        // Current partition start index
    int high;       // Current partition end index
    int pivotIndex; // Index of the pivot element (always at high)
    int i;          // Boundary of smaller elements (starts at low-1)
    int j;          // Current element being compared (starts at low)

    // State tracking
    bool partitioning;         // Whether we're in partitioning phase
    bool comparing;            // Whether we're comparing j-th element with pivot
    bool needsSwap;            // Whether current comparison needs a swap
    bool partitionComplete;    // Whether current partition is complete
    bool waitingForSwap;       // Whether we're waiting for user to perform swap
    bool pivotSwapping;        // Whether we're in pivot swapping phase
    bool hideOtherValues;      // Whether to show '?' for non-pivot values during pivot swap
    bool manualElevation;      // Whether user manually elevated rectangles with T key
    bool pivotInSortedRegion;  // Whether pivot has been placed in sorted region
    bool needsIncrementI;      // Whether i needs to be incremented before pivot swapping
    bool iIncrementedWaiting;  // Whether i was incremented and waiting for manual swap when i==j
    bool pivotNeedsIncrementI; // Whether i needs to be incremented before moving pivot to sorted region

    // Statistics
    int comparisons;         // Total comparisons made
    int swaps;               // Total swaps made
    int partitionsCompleted; // Number of partitions completed

    // Original values for comparison (before pickup/manipulation)
    int originalJValue;     // Value at j-th position
    int originalPivotValue; // Pivot value
    int originalIValue;     // Value at i-th position (for swap target)

    // Stack for recursive calls (manual stack implementation)
    int stackLow[MAX_ARRAY_SIZE];
    int stackHigh[MAX_ARRAY_SIZE];
    int stackTop;

    // Track positions with completed pivots (in their final sorted position)
    bool completedPivots[MAX_ARRAY_SIZE];

    // Animation system for smooth pivot transitions
    struct
    {
        bool animating[MAX_ARRAY_SIZE];   // Whether each position is currently being animated
        float startTime[MAX_ARRAY_SIZE];  // When each animation started
        float duration[MAX_ARRAY_SIZE];   // Total animation duration in seconds for each position
        float startY[MAX_ARRAY_SIZE];     // Starting Y position for each position
        float targetY[MAX_ARRAY_SIZE];    // Target Y position for each position
        float currentY[MAX_ARRAY_SIZE];   // Current animated Y position for each position
        bool isElevating[MAX_ARRAY_SIZE]; // Whether each position is elevating (true) or dropping (false)
        float bounceHeight;               // Additional bounce height for emphasis
        int bounceCount[MAX_ARRAY_SIZE];  // Number of bounces remaining for each position
        float bounceDecay;                // Decay factor for bounce amplitude

        // Trail effect for falling pivots
        struct
        {
            Vector2 positions[10]; // Trail positions (circular buffer)
            float alphas[10];      // Alpha values for trail fade
            int currentIndex;      // Current position in circular buffer
            float lastUpdateTime;  // Last time trail was updated
        } trail[MAX_ARRAY_SIZE];
    } pivotAnimation;

} QuickSortData;

// Quick Sort function declarations
void QuickSortInit(GameData *game);
void QuickSortUpdate(GameData *game);
void QuickSortRender(GameData *game);
void QuickSortGetStats(GameData *game, AlgorithmStats *stats);
void QuickSortCleanup(GameData *game);
bool QuickSortIsComplete(GameData *game);
void QuickSortResetLevel(GameData *game, int level);

// Utility function for UI rendering
bool QuickSortShouldHideValue(GameData *game, int position);
bool QuickSortIsCompletedPivot(GameData *game, int position);
bool QuickSortIsAnimating(GameData *game, int position);
bool QuickSortIsSwappingElement(GameData *game, int position);

// Animation helper functions
void StartPivotElevationAnimation(GameData *game, int position);
void StartPivotFallingAnimation(GameData *game, int position);
void UpdatePivotAnimation(GameData *game);
float GetAnimatedPivotY(GameData *game, int position);

// Player physics helper
void QuickSortHandlePlayerFallWithCompletion(GameData *game);

#endif // QUICK_SORT_H