#include "bubble_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// Helper function to get which platform player is on (-1 if none)
// static int GetPlayerPlatform(GameData* game) {
//     for (int i = 0; i < game->arraySize; i++) {
//         Rectangle platform = game->platforms[i];
//         // Check if player is standing on platform (matching reference code logic)
//         if (game->player.y + game->player.height <= platform.y + 10 &&
//             game->player.y + game->player.height >= platform.y - 10 &&
//             game->player.x + game->player.width > platform.x + 8 && 
//             game->player.x < platform.x + platform.width - 8 &&
//             game->isOnGround) {
//             return i;
//         }
//     }
//     return -1;
// }
// static int GetPlayerPlatform(GameData* game) {
//     for (int i = 0; i < game->arraySize; i++) {
//         Rectangle platform = game->platforms[i];
//         // EXACT SAME COLLISION LOGIC AS PLAYER PHYSICS
//         if (game->player.y + game->player.height <= platform.y + 10 &&
//             game->player.y + game->player.height >= platform.y - 10 &&
//             game->player.x + game->player.width > platform.x + 8 && 
//             game->player.x < platform.x + platform.width - 8) {
//             printf("GetPlayerPlatform: Player is on platform %d\n", i);
//             return i;
//         }
//     }
//     printf("GetPlayerPlatform: Player not on any platform\n");
//     return -1;
// }

static int GetPlayerPlatform(GameData* game) {
    for (int i = 0; i < game->arraySize; i++) {
        Rectangle platform = game->platforms[i];
        
        // Check if player is landing on top of platform (MATCH WORKING DEMO EXACTLY)
        if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
            game->player.y + game->player.height + game->velocity.y >= platform.y &&
            game->player.x + game->player.width > platform.x + 8 && 
            game->player.x < platform.x + platform.width - 8) {
            
            game->player.y = platform.y - game->player.height;
            game->velocity.y = 0;
            game->isOnGround = true;
            // printf("Player landed on platform %d\n", i);
            return i;
        }
    }

    return -1;
}


void BubbleSortInit(GameData* game) {
    BubbleSortData* data = (BubbleSortData*)malloc(sizeof(BubbleSortData));
    if (!data) {
        printf("Error: Failed to allocate memory for BubbleSortData\n");
        return;
    }
    
    data->currentI = 0;
    data->currentJ = 0;
    data->comparing = false;
    data->swapping = false;
    data->comparisons = 0;
    data->swaps = 0;
    data->sortedUpTo = -1;  // No elements sorted initially
    
    game->algorithmData = data;
    printf("Bubble Sort initialized\n");
}


void BubbleSortUpdate(GameData* game) {
    BubbleSortData* data = (BubbleSortData*)game->algorithmData;
    if (!data) return;
   
    
  
    // Check what key is being pressed (if any) helper func
    // for (int key = 32; key < 127; key++) {  // Printable ASCII range
    //     if (IsKeyPressed(key)) {
    //         printf("*** KEY PRESSED: %d (char: %c) ***\n", key, key);
    //     }
    // }
    

   
    // Handle interaction key input for manual bubble sort interactions
    // Using  F key 
    if (IsKeyPressed(KEY_F)) {
        int onSquare = GetPlayerPlatform(game);          /* platform player stands on */
        if (onSquare < 0) {                              /* not on any box → ignore */
            printf("Not on a platform – nothing happens\n");
            return;
        }

        /* ---- 1.  pick-up from current box --------------------------------- */
        if (!game->carrying && game->array[onSquare] != 0) {
            game->playerNumber = game->array[onSquare];
            game->array[onSquare] = 0;
            game->carrying = true;
            data->comparisons++;
            printf("Picked up %d from platform %d\n", game->playerNumber, onSquare);
            return;
        }

        /* ---- 2.  place into empty current box ---------------------------- */
        if (game->carrying && game->array[onSquare] == 0) {
            game->array[onSquare] = game->playerNumber;
            game->playerNumber = 0;
            game->carrying = false;
            printf("Placed %d on platform %d\n", game->array[onSquare], onSquare);
            return;
        }

        /* ---- 3.  swap with adjacent box (ASCENDING order) ---------------- */
        if (game->carrying && game->array[onSquare] != 0) {
            /* find the empty box index once */
            int emptyIndex = -1;
            for (int i = 0; i < game->arraySize; ++i) {
                if (game->array[i] == 0) { emptyIndex = i; break; }
            }
            if (emptyIndex == -1) {          /* no empty box → nothing to do */
                printf("No empty box – cannot swap\n");
                return;
            }

            /* adjacent check */
            if (onSquare == emptyIndex - 1 || onSquare == emptyIndex + 1) {
                int temp = game->array[onSquare];
                bool correct = false;

                if (emptyIndex > onSquare) {          /* swapping right */
                    if (game->playerNumber < game->array[onSquare]) correct = true;
                } else {                                /* swapping left */
                    if (game->playerNumber > game->array[onSquare]) correct = true;
                }

                if (correct) {
                    game->array[onSquare] = game->playerNumber;
                    game->playerNumber = temp;
                    data->swaps++;
                    printf("Adjacent swap – correct (ascending)\n");
                } else {
                    /* wrong swap */
                    game->hearts--;
                    printf("Adjacent swap – WRONG (ascending), hearts left: %d\n", game->hearts);
                    if (game->hearts <= 0) ChangeState(STATE_GAME_OVER);
                }
            } else {
                printf("Not adjacent – swap refused\n");
            }
        }
    }


   
    // Update sorted portion tracking
    data->sortedUpTo = -1;
    for (int i = 0; i < game->arraySize - 1; i++) {
        if (game->array[i] != 0 && game->array[i+1] != 0 && game->array[i] <= game->array[i+1]) {
            data->sortedUpTo = i;
        } else {
            break;
        }
    }
    
    // Check for completion - all elements must be in ascending order and no empty boxes
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

void BubbleSortRender(GameData* game) {
    BubbleSortData* data = (BubbleSortData*)game->algorithmData;
    if (!data) return;
    
    // Draw algorithm-specific UI
    char statsText[128];
    sprintf(statsText, "Interactions: %d | Swaps: %d", data->comparisons, data->swaps);
    DrawText(statsText, 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    
    // Show instructions based on player state
    if (game->carrying) {
        DrawText("Press F on a box to place or swap numbers", 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
        DrawText("Goal: Sort numbers in ascending order (1, 2, 3, 4...)", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    } else {
        DrawText("Press F on a box with a number to pick it up", 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
        DrawText("Goal: Sort numbers in ascending order (1, 2, 3, 4...)", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    }
    
    // Highlight sorted portion in light green
    for (int i = 0; i <= data->sortedUpTo; i++) {
        if (i < game->arraySize && game->array[i] != 0) {
            DrawRectangleLinesEx(game->platforms[i], 3, GAME_SORTED);
        }
    }
    
    // Highlight current player platform if they can interact
    int playerPlatform = GetPlayerPlatform(game);
    // printf("platform:  %d\n", playerPlatform);
    if (playerPlatform >= 0) {
        Color highlightColor = GAME_HIGHLIGHT;
        if (game->carrying && game->array[playerPlatform] != 0) {
            highlightColor = GAME_SELECTED;  // Different color for swap interaction
        }
        DrawRectangleLinesEx(game->platforms[playerPlatform], 2, highlightColor);
    }
    
    // Draw carried number on player character (properly centered)
    if (game->carrying && game->playerNumber > 0) {
        char numText[16];
        sprintf(numText, "%d", game->playerNumber);
        
        int textWidth = MeasureText(numText, FONT_SIZE_BODY);
        int textX = game->player.x + (game->player.width - textWidth) / 2;
        int textY = game->player.y + (game->player.height - FONT_SIZE_BODY) / 2;
        DrawText(numText, textX, textY, FONT_SIZE_BODY, (Color){0, 0, 0, 255}); // Black text for visibility
    }
}

void BubbleSortCleanup(GameData* game) {
    if (game && game->algorithmData) {
        free(game->algorithmData);
        game->algorithmData = NULL;
        
        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Bubble Sort cleaned up\n");
}

bool BubbleSortIsComplete(GameData* game) {
    // Check if all boxes are filled (no empty boxes)
    for (int i = 0; i < game->arraySize; i++) {
        if (game->array[i] == 0) {
            return false;
        }
    }
    
    // Check if array is sorted in ascending order
    return IsArraySorted(game->array, game->arraySize, true);
}

void BubbleSortResetLevel(GameData* game, int level) {
    // Reset algorithm-specific data
    BubbleSortData* data = (BubbleSortData*)game->algorithmData;
    if (data) {
        data->currentI = 0;
        data->currentJ = 0;
        data->comparing = false;
        data->swapping = false;
        data->comparisons = 0;
        data->swaps = 0;
        data->sortedUpTo = -1;
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
    
    printf("Bubble Sort level %d reset with %d elements\n", level, game->arraySize);
}