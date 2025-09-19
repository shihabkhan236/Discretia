#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>

// Placeholder implementation for Merge Sort
typedef struct {
    int currentIndex;
    int comparisons;
    int swaps;
} MergeSortData;

void MergeSortInit(GameData* game) {
    MergeSortData* data = (MergeSortData*)malloc(sizeof(MergeSortData));
    data->currentIndex = 0;
    data->comparisons = 0;
    data->swaps = 0;
    game->algorithmData = data;
    printf("Merge Sort initialized\n");
}

void MergeSortUpdate(GameData* game) { (void)game; }

void MergeSortRender(GameData* game) {
    DrawText("Merge Sort - Not implemented yet", 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    (void)game;
}

void MergeSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
    }
    printf("Merge Sort cleaned up\n");
}

bool MergeSortIsComplete(GameData* game) {
    return IsArraySorted(game->array, game->arraySize, true);
}

void MergeSortResetLevel(GameData* game, int level) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
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
    printf("Merge Sort level %d reset\n", level);
}