#ifndef SELECTION_SORT_H
#define SELECTION_SORT_H

#include "../core/game.h"

// Selection Sort specific data
typedef struct {
    int sortedBoundary;      // Index separating sorted/unsorted sections
    int currentSearchIndex;  // Current position when finding minimum
    int minimumIndex;        // Index of found minimum element
    bool findingMinimum;     // State: searching for minimum
    bool confirmingMinimum;  // State: player confirming minimum selection
    bool swapping;           // State: performing swap operation
    int comparisons;         // Statistics tracking
    int swaps;              // Statistics tracking
    int mistakes;           // Wrong minimum selections
} SelectionSortData;

// Selection Sort function declarations
void SelectionSortInit(GameData* game);
void SelectionSortUpdate(GameData* game);
void SelectionSortRender(GameData* game);
void SelectionSortGetStats(GameData* game, AlgorithmStats* stats);
void SelectionSortCleanup(GameData* game);
bool SelectionSortIsComplete(GameData* game);
void SelectionSortResetLevel(GameData* game, int level);

#endif // SELECTION_SORT_H