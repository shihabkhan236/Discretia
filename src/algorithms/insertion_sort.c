#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>

// Placeholder implementation for Insertion Sort
typedef struct {
    int currentIndex;
    int comparisons;
    int swaps;
} InsertionSortData;

void InsertionSortInit(GameData* game) {
    InsertionSortData* data = (InsertionSortData*)malloc(sizeof(InsertionSortData));
    data->currentIndex = 0;
    data->comparisons = 0;
    data->swaps = 0;
    game->algorithmData = data;
    printf("Insertion Sort initialized\n");
}

void InsertionSortUpdate(GameData* game) { (void)game; }

void InsertionSortRender(GameData* game) {
    DrawText("Insertion Sort - Not implemented yet", 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    (void)game;
}

void InsertionSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
    }
    printf("Insertion Sort cleaned up\n");
}

bool InsertionSortIsComplete(GameData* game) {
    return IsArraySorted(game->array, game->arraySize, true);
}

void InsertionSortResetLevel(GameData* game, int level) {
    InsertionSortData* data = (InsertionSortData*)game->algorithmData;
    if (data) {
        data->currentIndex = 0;
        data->comparisons = 0;
        data->swaps = 0;
    }
    
    switch (level) {
        case 0:
            game->arraySize = 4;
            int tutorial[] = {4, 2, 3, 1};
            for (int i = 0; i < game->arraySize; i++) {
                game->array[i] = tutorial[i];
            }
            break;
        default:
            game->arraySize = 4 + level;
            if (game->arraySize > MAX_ARRAY_SIZE) game->arraySize = MAX_ARRAY_SIZE;
            for (int i = 0; i < game->arraySize; i++) {
                game->array[i] = i + 1;
            }
            ShuffleArray(game->array, game->arraySize);
            break;
    }
    printf("Insertion Sort level %d reset\n", level);
}