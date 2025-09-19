#include "selection_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>

// Selection Sort specific data
typedef struct {
    int sortedBoundary;
    int selectedIndex;
    bool findingMin;
    bool moving;
    int comparisons;
    int swaps;
} SelectionSortData;

void SelectionSortInit(GameData* game) {
    SelectionSortData* data = (SelectionSortData*)malloc(sizeof(SelectionSortData));
    data->sortedBoundary = 0;
    data->selectedIndex = -1;
    data->findingMin = true;
    data->moving = false;
    data->comparisons = 0;
    data->swaps = 0;
    
    game->algorithmData = data;
    printf("Selection Sort initialized\n");
}

void SelectionSortUpdate(GameData* game) {
    // Placeholder implementation
    (void)game;
}

void SelectionSortRender(GameData* game) {
    // Placeholder implementation
    DrawText("Selection Sort - Not implemented yet", 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    (void)game;
}

void SelectionSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
    }
    printf("Selection Sort cleaned up\n");
}

bool SelectionSortIsComplete(GameData* game) {
    return IsArraySorted(game->array, game->arraySize, true);
}

void SelectionSortResetLevel(GameData* game, int level) {
    // Reset algorithm-specific data
    SelectionSortData* data = (SelectionSortData*)game->algorithmData;
    if (data) {
        data->sortedBoundary = 0;
        data->selectedIndex = -1;
        data->findingMin = true;
        data->moving = false;
        data->comparisons = 0;
        data->swaps = 0;
    }
    
    // Generate level array (same as bubble sort for now)
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
    
    printf("Selection Sort level %d reset\n", level);
}