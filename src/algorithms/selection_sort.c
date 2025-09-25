#include "selection_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



void SelectionSortInit(GameData* game) {
    SelectionSortData* data = (SelectionSortData*)malloc(sizeof(SelectionSortData));
    if (!data) {
        printf("Error: Failed to allocate memory for SelectionSortData\n");
        return;
    }
    
    data->sortedBoundary = 0;
    data->currentSearchIndex = 0;
    data->minimumIndex = -1;
    data->findingMinimum = true;
    data->confirmingMinimum = false;
    data->swapping = false;
    data->comparisons = 0;
    data->swaps = 0;
    data->mistakes = 0;
    
    game->algorithmData = data;
    printf("Selection Sort initialized\n");
}

void SelectionSortUpdate(GameData* game) {
    SelectionSortData* data = (SelectionSortData*)game->algorithmData;
    if (!data) return;
    
    // Handle interaction key input for Selection Sort interactions
    if (IsKeyPressed(KEY_F)) {
        int onSquare = GetPlayerPlatform(game);
        if (onSquare < 0) {
            printf("Not on a platform – nothing happens\n");
            return;
        }
        
        // Phase 1: Finding minimum in unsorted section
        if (data->findingMinimum && !game->carrying) {
            // Only allow picking up from unsorted section
            if (onSquare >= data->sortedBoundary && game->array[onSquare] != 0) {
                // Check if this is the actual minimum in unsorted section
                int actualMin = game->array[data->sortedBoundary];
                int actualMinIndex = data->sortedBoundary;
                
                for (int i = data->sortedBoundary; i < game->arraySize; i++) {
                    if (game->array[i] != 0 && game->array[i] < actualMin) {
                        actualMin = game->array[i];
                        actualMinIndex = i;
                    }
                }
                
                if (onSquare == actualMinIndex) {
                    // Correct minimum selection
                    game->playerNumber = game->array[onSquare];
                    game->array[onSquare] = 0;
                    game->carrying = true;
                    data->minimumIndex = onSquare;
                    data->findingMinimum = false;
                    data->swapping = true;
                    data->comparisons++;
                    printf("Correct minimum %d selected from platform %d\n", game->playerNumber, onSquare);
                } else {
                    // Wrong minimum selection
                    game->hearts--;
                    data->mistakes++;
                    printf("Wrong minimum selection! Hearts left: %d\n", game->hearts);
                    if (game->hearts <= 0) ChangeState(STATE_GAME_OVER);
                }
            }
        }
        // Phase 2: Swapping minimum with sorted boundary
        else if (data->swapping && game->carrying) {
            if (onSquare == data->sortedBoundary) {
                // Swap with sorted boundary position
                if (game->array[onSquare] == 0) {
                    // Place in empty boundary position
                    game->array[onSquare] = game->playerNumber;
                    game->playerNumber = 0;
                    game->carrying = false;
                } else {
                    // Swap with element at boundary
                    int temp = game->array[onSquare];
                    game->array[onSquare] = game->playerNumber;
                    game->playerNumber = temp;
                }
                
                data->swaps++;
                data->sortedBoundary++;
                data->swapping = false;
                
                // Check if we're done or continue finding next minimum
                if (data->sortedBoundary >= game->arraySize - 1) {
                    // Array is sorted
                    if (game->carrying) {
                        // Place last element
                        for (int i = 0; i < game->arraySize; i++) {
                            if (game->array[i] == 0) {
                                game->array[i] = game->playerNumber;
                                game->playerNumber = 0;
                                game->carrying = false;
                                break;
                            }
                        }
                    }
                } else {
                    data->findingMinimum = true;
                }
                
                printf("Swapped minimum to sorted position %d\n", data->sortedBoundary - 1);
            } else {
                printf("Must swap with sorted boundary position %d\n", data->sortedBoundary);
            }
        }
        // Allow placing carried element back
        else if (game->carrying && game->array[onSquare] == 0) {
            game->array[onSquare] = game->playerNumber;
            game->playerNumber = 0;
            game->carrying = false;
            data->findingMinimum = true;
            data->swapping = false;
            printf("Placed %d back on platform %d\n", game->array[onSquare], onSquare);
        }
    }
    
    // Check for completion
    bool allFilled = true;
    for (int i = 0; i < game->arraySize; i++) {
        if (game->array[i] == 0) {
            allFilled = false;
            break;
        }
    }
    
    if (allFilled && IsArraySorted(game->array, game->arraySize, true)) {
        ChangeState(STATE_LEVEL_COMPLETE);
    }
}

void SelectionSortGetStats(GameData* game, AlgorithmStats* stats) {
    SelectionSortData* data = (SelectionSortData*)game->algorithmData;
    if (!data) return;
    
    // Primary stat: Comparisons, swaps, mistakes
    sprintf(stats->primaryStat, "Comparisons: %d | Swaps: %d | Mistakes: %d", 
            data->comparisons, data->swaps, data->mistakes);
    
    // Secondary stat and instructions based on algorithm state
    stats->hasInstruction = true;
    if (data->findingMinimum && !game->carrying) {
        sprintf(stats->secondaryStat, "Unsorted section: positions %d to %d", 
                data->sortedBoundary, game->arraySize - 1);
        strcpy(stats->instructionText, "Find and press F on the MINIMUM element in the unsorted section");
        stats->instructionColor = UI_TEXT_PRIMARY;
    } else if (data->swapping && game->carrying) {
        sprintf(stats->secondaryStat, "Swapping minimum to position %d", data->sortedBoundary);
        sprintf(stats->instructionText, "Press F on position %d to swap minimum into sorted section", 
                data->sortedBoundary);
        stats->instructionColor = GAME_COMPARING;
    } else if (game->carrying) {
        strcpy(stats->secondaryStat, "Placing element back");
        strcpy(stats->instructionText, "Press F on an empty box to place the number");
        stats->instructionColor = UI_TEXT_PRIMARY;
    } else {
        strcpy(stats->secondaryStat, "");
        stats->hasInstruction = false;
    }
    
    strcpy(stats->goalText, "Goal: Sort numbers in ascending order (1, 2, 3, 4...)");
}

void SelectionSortRender(GameData* game) {
    SelectionSortData* data = (SelectionSortData*)game->algorithmData;
    if (!data) return;
    
    // Only render visual highlights and game objects, no text UI
    
    // Highlight sorted section in green
    for (int i = 0; i < data->sortedBoundary; i++) {
        if (game->array[i] != 0) {
            DrawRectangleLinesEx(game->platforms[i], 3, GAME_SORTED);
        }
    }
    
    // Show correct positions when H key is held
    if (IsKeyDown(KEY_H)) {
        int target[MAX_ARRAY_SIZE];
        for (int i = 0; i < game->arraySize; i++) target[i] = i + 1;
        
        // Mark boxes that are already in final place
        for (int i = 0; i < game->arraySize; i++) {
            if (game->array[i] != 0 && game->array[i] == target[i]) {
                DrawRectangleLinesEx(game->platforms[i], 3, GAME_SORTED);
            }
        }
    }
}

void SelectionSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
        
        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Selection Sort cleaned up\n");
}

bool SelectionSortIsComplete(GameData* game) {
    // Check if all boxes are filled (no empty boxes)
    for (int i = 0; i < game->arraySize; i++) {
        if (game->array[i] == 0) {
            return false;
        }
    }
    
    // Check if array is sorted in ascending order
    return IsArraySorted(game->array, game->arraySize, true);
}

void SelectionSortResetLevel(GameData* game, int level) {
    // Reset algorithm-specific data
    SelectionSortData* data = (SelectionSortData*)game->algorithmData;
    if (data) {
        data->sortedBoundary = 0;
        data->currentSearchIndex = 0;
        data->minimumIndex = -1;
        data->findingMinimum = true;
        data->confirmingMinimum = false;
        data->swapping = false;
        data->comparisons = 0;
        data->swaps = 0;
        data->mistakes = 0;
    }
    
    // Reset player carrying state
    game->carrying = false;
    game->playerNumber = 0;
    
    // Generate level array
    switch (level) {
        case 0: // Tutorial - simple 4-element array
            game->arraySize = 4;
            int tutorial[] = {4, 2, 3, 1};
            for (int i = 0; i < game->arraySize; i++) {
                game->array[i] = tutorial[i];
            }
            break;
        case 1: // Easy - 5 elements
            game->arraySize = 5;
            for (int i = 0; i < game->arraySize; i++) {
                game->array[i] = i + 1;
            }
            ShuffleArray(game->array, game->arraySize);
            break;
        default: // Progressive difficulty
            game->arraySize = 4 + level;
            if (game->arraySize > MAX_ARRAY_SIZE) game->arraySize = MAX_ARRAY_SIZE;
            
            // Generate sequential numbers and shuffle
            for (int i = 0; i < game->arraySize; i++) {
                game->array[i] = i + 1;
            }
            ShuffleArray(game->array, game->arraySize);
            break;
    }
    
    printf("Selection Sort level %d reset with %d elements\n", level, game->arraySize);
}