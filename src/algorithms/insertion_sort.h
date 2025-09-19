#ifndef INSERTION_SORT_H
#define INSERTION_SORT_H

#include "../core/game.h"

// Insertion Sort function declarations
void InsertionSortInit(GameData* game);
void InsertionSortUpdate(GameData* game);
void InsertionSortRender(GameData* game);
void InsertionSortCleanup(GameData* game);
bool InsertionSortIsComplete(GameData* game);
void InsertionSortResetLevel(GameData* game, int level);

#endif // INSERTION_SORT_H