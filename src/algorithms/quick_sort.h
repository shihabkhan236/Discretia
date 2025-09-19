#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include "../core/game.h"

// Quick Sort function declarations
void QuickSortInit(GameData* game);
void QuickSortUpdate(GameData* game);
void QuickSortRender(GameData* game);
void QuickSortCleanup(GameData* game);
bool QuickSortIsComplete(GameData* game);
void QuickSortResetLevel(GameData* game, int level);

#endif // QUICK_SORT_H