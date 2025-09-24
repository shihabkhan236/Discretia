#ifndef INSERTION_SORT_H
#define INSERTION_SORT_H

#include "../core/game.h"

// Insertion Sort specific data
typedef struct
{
    int currentIndex;      // Current element being inserted (i)
    int insertionPosition; // Position where element will be inserted
    bool pickingElement;   // State: picking up current element
    bool comparing;        // State: comparing with sorted elements
    bool shifting;         // State: shifting elements right
    bool inserting;        // State: placing element in position
    int comparisons;       // Statistics tracking
    int shifts;            // Element shift operations
    int mistakes;          // Wrong placement attempts
    bool canSkip;          // Whether current element can be skipped (already in correct position)

    // Visual comparison tracking (like bubble sort)
    int leftIndex;       // Index of left element being compared
    int rightIndex;      // Index of right element being compared
    int originalLeft;    // Original value at left position (for comparison when picked up)
    int originalRight;   // Original value at right position (for comparison when picked up)
    bool showComparison; // Whether to highlight the comparison
} InsertionSortData;

// Insertion Sort function declarations
void InsertionSortInit(GameData *game);
void InsertionSortUpdate(GameData *game);
void InsertionSortRender(GameData *game);
void InsertionSortCleanup(GameData *game);
bool InsertionSortIsComplete(GameData *game);
void InsertionSortResetLevel(GameData *game, int level);

#endif // INSERTION_SORT_H