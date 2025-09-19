#include "quick_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>

// Placeholder implementation for Quick Sort
typedef struct {
    int currentIndex;
    int comparisons;
    int swaps;
} QuickSortData;

void QuickSortInit(GameData* game) {
    QuickSortData* data = (QuickSortData*)malloc(sizeof(QuickSortData));
    data->currentIndex = 0;
    data->comparisons = 0;
    data->swaps = 0;
    game->algorithmData = data;
    printf("Quick Sort initialized\n");
}

void QuickSortUpdate(GameData* game) { (void)game; }

void QuickSortRender(GameData* game) {
    DrawText("Quick Sort - Not implemented yet", 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    (void)game;
}

void QuickSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
    }
    printf("Quick Sort cleaned up\n");
}

bool QuickSortIsComplete(GameData* game) {
    return IsArraySorted(game->array, game->arraySize, true);
}

void QuickSortResetLevel(GameData* game, int level) {
    QuickSortData* data = (QuickSortData*)game->algorithmData;
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
    printf("Quick Sort level %d reset\n", level);
}