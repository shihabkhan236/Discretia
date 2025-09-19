#ifndef BUBBLE_SORT_H
#define BUBBLE_SORT_H

#include "../core/game.h"

// Bubble Sort function declarations
void BubbleSortInit(GameData* game);
void BubbleSortUpdate(GameData* game);
void BubbleSortRender(GameData* game);
void BubbleSortCleanup(GameData* game);
bool BubbleSortIsComplete(GameData* game);
void BubbleSortResetLevel(GameData* game, int level);

#endif // BUBBLE_SORT_H