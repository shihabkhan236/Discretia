#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include "../core/game.h"

// Merge Sort function declarations
void MergeSortInit(GameData *game);
void MergeSortUpdate(GameData *game);
void MergeSortRender(GameData *game);
void MergeSortGetStats(GameData *game, AlgorithmStats *stats);
void MergeSortCleanup(GameData *game);
bool MergeSortIsComplete(GameData *game);
void MergeSortResetLevel(GameData *game, int level);

// Additional collision detection for subarrays
int MergeSortGetPlayerPlatform(GameData *game);

// Get carried number for player display
int MergeSortGetCarriedNumber(GameData *game);
bool MergeSortIsCarryingNumber(GameData *game);

#endif // MERGE_SORT_H