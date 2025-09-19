#ifndef SELECTION_SORT_H
#define SELECTION_SORT_H

#include "../core/game.h"

// Selection Sort function declarations
void SelectionSortInit(GameData* game);
void SelectionSortUpdate(GameData* game);
void SelectionSortRender(GameData* game);
void SelectionSortCleanup(GameData* game);
bool SelectionSortIsComplete(GameData* game);
void SelectionSortResetLevel(GameData* game, int level);

#endif // SELECTION_SORT_H