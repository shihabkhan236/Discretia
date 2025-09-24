#ifndef BUBBLE_SORT_H
#define BUBBLE_SORT_H

#include "../core/game.h"

// Bubble Sort specific constants
#define INTERACTION_RANGE 10 // Pixels within platform to allow interaction
// Bubble Sort specific data
typedef struct
{
    int currentI;      // Current outer loop index (for tracking progress)
    int currentJ;      // Current inner loop index (for tracking progress)
    bool comparing;    // Whether currently in comparison state
    bool swapping;     // Whether currently performing a swap
    int comparisons;   // Total number of comparisons made
    int swaps;         // Total number of swaps performed
    int originalLeft;  // Original value at currentJ position (before any pickup)
    int originalRight; // Original value at currentJ+1 position (before any pickup)
} BubbleSortData;

// Bubble Sort function declarations
void BubbleSortInit(GameData *game);
void BubbleSortUpdate(GameData *game);
void BubbleSortRender(GameData *game);
void BubbleSortCleanup(GameData *game);
bool BubbleSortIsComplete(GameData *game);
void BubbleSortResetLevel(GameData *game, int level);

#endif // BUBBLE_SORT_H