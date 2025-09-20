#include "insertion_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void InsertionSortInit(GameData* game) {
    InsertionSortData* data = (InsertionSortData*)malloc(sizeof(InsertionSortData));
    if (!data) {
        printf("Error: Failed to allocate memory for InsertionSortData\n");
        return;
    }
    
    data->currentIndex = 1; // Start from second element
    data->insertionPosition = -1;
    data->pickingElement = true;
    data->comparing = false;
    data->shifting = false;
    data->inserting = false;
    data->comparisons = 0;
    data->shifts = 0;
    data->mistakes = 0;
    
    game->algorithmData = data;
    printf("Insertion Sort initialized\n");
}


// FIXED InsertionSortUpdate - only advances when element is in correct position
// void InsertionSortUpdate(GameData* game) {
//     InsertionSortData* data = (InsertionSortData*)game->algorithmData;
//     if (!data) return;
    
//     if (IsKeyPressed(KEY_F)) {
//         int onSquare = GetPlayerPlatform(game);
//         printf("F pressed! On platform: %d, Carrying: %s, Current index: %d\n", 
//                onSquare, game->carrying ? "YES" : "NO", data->currentIndex);
        
//         if (onSquare < 0) {
//             printf("Not on any platform\n");
//             return;
//         }
        
//         // STEP 1: Pick up the current element to be inserted
//         if (!game->carrying && data->currentIndex < game->arraySize) {
//             if (onSquare == data->currentIndex && game->array[onSquare] != 0) {
//                 game->playerNumber = game->array[onSquare];
//                 game->array[onSquare] = 0;  // Create empty space
//                 game->carrying = true;
//                 data->pickingElement = false;
//                 data->comparing = true;
//                 printf("✓ Picked up element %d from position %d\n", game->playerNumber, onSquare);
//             } else {
//                 printf("✗ Must pick up element from position %d (you're at %d)\n", data->currentIndex, onSquare);
//             }
//         }
//         // STEP 2: Handle placement/swapping while carrying
//         else if (game->carrying) {
//             // Find where the empty space currently is
//             int emptyIndex = -1;
//             for (int i = 0; i < game->arraySize; i++) {
//                 if (game->array[i] == 0) {
//                     emptyIndex = i;
//                     break;
//                 }
//             }
            
//             if (emptyIndex == -1) {
//                 printf("Error: No empty space found!\n");
//                 return;
//             }
            
//             if (onSquare == emptyIndex) {
//                 // Place in empty space
//                 game->array[onSquare] = game->playerNumber;
//                 game->playerNumber = 0;
//                 game->carrying = false;
                
//                 // FIXED: Check if the ENTIRE processed section (0 to currentIndex) is sorted
//                 bool processingComplete = true;
                
//                 // Check if the range [0...currentIndex] is sorted
//                 for (int i = 0; i < data->currentIndex; i++) {
//                     for (int j = i + 1; j <= data->currentIndex; j++) {
//                         if (game->array[i] > game->array[j]) {
//                             processingComplete = false;
//                             printf("Debug: Found out of order: array[%d]=%d > array[%d]=%d\n", 
//                                    i, game->array[i], j, game->array[j]);
//                             break;
//                         }
//                     }
//                     if (!processingComplete) break;
//                 }
                
//                 if (processingComplete) {
//                     // The section [0...currentIndex] is fully sorted, move to next element
//                     data->currentIndex++;
//                     data->pickingElement = true;
//                     data->comparing = false;
//                     printf("✓ Section [0...%d] is sorted! Moving to next element (%d)\n", 
//                            data->currentIndex - 1, data->currentIndex);
//                 } else {
//                     // Section not fully sorted - allow picking up again
//                     data->pickingElement = true;
//                     data->comparing = false;
//                     printf("⚠ Section not fully sorted. Continue inserting element %d.\n", data->currentIndex);
//                 }
//             }
//             else if (abs(onSquare - emptyIndex) == 1 && game->array[onSquare] != 0) {
//                 // Adjacent swap - this moves the empty space
//                 bool validMove = false;
                
//                 if (emptyIndex > onSquare) {
//                     // Moving empty space right (element moves left)
//                     if (game->playerNumber <= game->array[onSquare]) {
//                         validMove = true;
//                     }
//                 } else {
//                     // Moving empty space left (element moves right)  
//                     if (game->playerNumber >= game->array[onSquare]) {
//                         validMove = true;
//                     }
//                 }
                
//                 if (validMove) {
//                     // Perform the swap
//                     int temp = game->array[onSquare];
//                     game->array[onSquare] = game->playerNumber;
//                     game->playerNumber = temp;
//                     data->shifts++;
//                     data->comparisons++;
//                     printf("✓ Swapped elements (empty space moved)\n");
//                 } else {
//                     // Wrong move - penalty
//                     game->hearts--;
//                     data->mistakes++;
//                     printf("✗ Wrong swap direction! Hearts remaining: %d\n", game->hearts);
//                     if (game->hearts <= 0) {
//                         ChangeState(STATE_GAME_OVER);
//                     }
//                 }
//             } else {
//                 printf("✗ Can only place in empty space or swap with adjacent elements\n");
//             }
//         }
//     }
    
//     // Check completion
//     if (data->currentIndex >= game->arraySize) {
//         bool allFilled = true;
//         for (int i = 0; i < game->arraySize; i++) {
//             if (game->array[i] == 0) {
//                 allFilled = false;
//                 break;
//             }
//         }
        
//         if (allFilled && IsArraySorted(game->array, game->arraySize, true)) {
//             ChangeState(STATE_LEVEL_COMPLETE);
//         }
//     }
// }
void InsertionSortUpdate(GameData* game) {
    InsertionSortData* data = (InsertionSortData*)game->algorithmData;
    if (!data) return;
    
    if (IsKeyPressed(KEY_F)) {
        int onSquare = GetPlayerPlatform(game);
        printf("F pressed! On platform: %d, Carrying: %s, Current index: %d\n", 
               onSquare, game->carrying ? "YES" : "NO", data->currentIndex);
        
        if (onSquare < 0) {
            printf("Not on any platform\n");
            return;
        }
        
        // STEP 1: Pick up elements - FIXED to allow any element in processing range
        if (!game->carrying && data->currentIndex < game->arraySize) {
            bool canPickUp = false;
            
            // Allow picking up from the processing range [0...currentIndex]
            if (onSquare <= data->currentIndex && game->array[onSquare] != 0) {
                canPickUp = true;
                printf("✓ Picking up element %d from position %d (within processing range 0-%d)\n", 
                       game->array[onSquare], onSquare, data->currentIndex);
            }
            
            if (canPickUp) {
                game->playerNumber = game->array[onSquare];
                game->array[onSquare] = 0;  // Create empty space
                game->carrying = true;
                data->pickingElement = false;
                data->comparing = true;
                printf("✓ Picked up element %d from position %d\n", game->playerNumber, onSquare);
            } else {
                printf("✗ Can only pick up elements from positions 0-%d (you're at %d)\n", data->currentIndex, onSquare);
            }
        }
        // STEP 2: Handle placement/swapping while carrying
        else if (game->carrying) {
            // Find where the empty space currently is
            int emptyIndex = -1;
            for (int i = 0; i < game->arraySize; i++) {
                if (game->array[i] == 0) {
                    emptyIndex = i;
                    break;
                }
            }
            
            if (emptyIndex == -1) {
                printf("Error: No empty space found!\n");
                return;
            }
            
            if (onSquare == emptyIndex) {
                // Place in empty space
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = 0;
                game->carrying = false;
                
                // FIXED: Check if the ENTIRE processed section (0 to currentIndex) is sorted
                bool processingComplete = true;
                
                // Check if the range [0...currentIndex] is sorted
                for (int i = 0; i < data->currentIndex; i++) {
                    for (int j = i + 1; j <= data->currentIndex; j++) {
                        if (game->array[i] > game->array[j]) {
                            processingComplete = false;
                            printf("Debug: Found out of order: array[%d]=%d > array[%d]=%d\n", 
                                   i, game->array[i], j, game->array[j]);
                            break;
                        }
                    }
                    if (!processingComplete) break;
                }
                
                if (processingComplete) {
                    // The section [0...currentIndex] is fully sorted, move to next element
                    data->currentIndex++;
                    data->pickingElement = true;
                    data->comparing = false;
                    printf("✓ Section [0...%d] is sorted! Moving to next element (%d)\n", 
                           data->currentIndex - 1, data->currentIndex);
                } else {
                    // Section not fully sorted - allow picking up again from anywhere in range
                    data->pickingElement = true;
                    data->comparing = false;
                    printf("⚠ Section not fully sorted. You can pick up any element in range [0-%d].\n", data->currentIndex);
                }
            }
            else if (abs(onSquare - emptyIndex) == 1 && game->array[onSquare] != 0) {
                // Adjacent swap - this moves the empty space
                bool validMove = false;
                
                if (emptyIndex > onSquare) {
                    // Moving empty space right (element moves left)
                    if (game->playerNumber <= game->array[onSquare]) {
                        validMove = true;
                    }
                } else {
                    // Moving empty space left (element moves right)  
                    if (game->playerNumber >= game->array[onSquare]) {
                        validMove = true;
                    }
                }
                
                if (validMove) {
                    // Perform the swap
                    int temp = game->array[onSquare];
                    game->array[onSquare] = game->playerNumber;
                    game->playerNumber = temp;
                    data->shifts++;
                    data->comparisons++;
                    printf("✓ Swapped elements (empty space moved)\n");
                } else {
                    // Wrong move - penalty
                    game->hearts--;
                    data->mistakes++;
                    printf("✗ Wrong swap direction! Hearts remaining: %d\n", game->hearts);
                    if (game->hearts <= 0) {
                        ChangeState(STATE_GAME_OVER);
                    }
                }
            } else {
                printf("✗ Can only place in empty space or swap with adjacent elements\n");
            }
        }
    }
    
    // Check completion
    if (data->currentIndex >= game->arraySize) {
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
}

void InsertionSortRender(GameData* game) {
    InsertionSortData* data = (InsertionSortData*)game->algorithmData;
    if (!data) return;
    
    // Draw algorithm-specific UI
    char statsText[128];
    sprintf(statsText, "Comparisons: %d | Shifts: %d | Mistakes: %d", data->comparisons, data->shifts, data->mistakes);
    DrawText(statsText, 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    
    // Show instructions based on algorithm state
    if (data->pickingElement && !game->carrying && data->currentIndex < game->arraySize) {
        char pickText[64];
        sprintf(pickText, "Press F to pick up element at position %d", data->currentIndex);
        DrawText(pickText, 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
        DrawText("This element will be inserted into the sorted section", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    } else if (data->comparing && game->carrying) {
        DrawText("Move left through sorted section using adjacent swaps", 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
        DrawText("Press F on adjacent elements to swap, or empty box to place", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    } else if (game->carrying) {
        DrawText("Press F on an empty box to place the number", 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    }
    DrawText("Goal: Sort numbers in ascending order (1, 2, 3, 4...)", 20, 180, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    

     // FIXED: Only highlight as sorted if the range is actually sorted
    // Check each position in the processed section
    for (int i = 0; i < data->currentIndex && i < game->arraySize; i++) {
        bool positionCorrect = true;
        
        // Check if this position maintains sorted order with its neighbors
        if (i > 0 && game->array[i-1] > game->array[i]) positionCorrect = false;
        if (i < data->currentIndex - 1 && game->array[i] > game->array[i+1]) positionCorrect = false;
        
        if (positionCorrect && game->array[i] != 0) {
            DrawRectangleLinesEx(game->platforms[i], 3, GAME_SORTED); // Green for correctly sorted
        } else if (game->array[i] != 0) {
            DrawRectangleLinesEx(game->platforms[i], 3, (Color){255, 255, 0, 255}); // Yellow for needs sorting
        }
    }
 
    
    // Highlight current element to be inserted
    if (data->currentIndex < game->arraySize && data->pickingElement) {
        DrawRectangleLinesEx(game->platforms[data->currentIndex], 3, (Color){255, 165, 0, 255}); // Orange for current element
    }
    
    // Highlight current player platform
    int playerPlatform = GetPlayerPlatform(game);
    if (playerPlatform >= 0) {
        Color highlightColor = GAME_HIGHLIGHT;
        if (data->pickingElement && playerPlatform == data->currentIndex) {
            highlightColor = GAME_SELECTED; // Special color for element to pick
        } else if (data->comparing && game->carrying) {
            // Find empty box
            int emptyIndex = -1;
            for (int i = 0; i < game->arraySize; i++) {
                if (game->array[i] == 0) {
                    emptyIndex = i;
                    break;
                }
            }
            if (playerPlatform == emptyIndex || abs(playerPlatform - emptyIndex) == 1) {
                highlightColor = (Color){0, 255, 255, 255}; // Cyan for valid insertion moves
            }
        }
        DrawRectangleLinesEx(game->platforms[playerPlatform], 2, highlightColor);
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

void InsertionSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
        
        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Insertion Sort cleaned up\n");
}

bool InsertionSortIsComplete(GameData* game) {
    // Check if all boxes are filled (no empty boxes)
    for (int i = 0; i < game->arraySize; i++) {
        if (game->array[i] == 0) {
            return false;
        }
    }
    
    // Check if array is sorted in ascending order
    return IsArraySorted(game->array, game->arraySize, true);
}

void InsertionSortResetLevel(GameData* game, int level) {
    // Reset algorithm-specific data
    InsertionSortData* data = (InsertionSortData*)game->algorithmData;
    if (data) {
        data->currentIndex = 1; // Start from second element
        data->insertionPosition = -1;
        data->pickingElement = true;
        data->comparing = false;
        data->shifting = false;
        data->inserting = false;
        data->comparisons = 0;
        data->shifts = 0;
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
    
    printf("Insertion Sort level %d reset with %d elements\n", level, game->arraySize);
}