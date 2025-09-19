#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Bubble Sort specific constants
#define INTERACTION_RANGE 10  // Pixels within platform to allow interaction

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
// claude fix 
static int GetPlayerPlatform(GameData* game) {
    // Simple approach: check if player is overlapping with any platform
    Rectangle playerRect = game->player;
    
    for (int i = 0; i < game->arraySize; i++) {
        Rectangle platform = game->platforms[i];
        
        // Expand the platform detection area to make it easier to interact
        Rectangle expandedPlatform = {
            platform.x - 20,
            platform.y - 20,
            platform.width + 40,
            platform.height + 40
        };
        
        // Check if player rectangle overlaps with expanded platform
        if (CheckCollisionRecs(playerRect, expandedPlatform)) {
            printf("Player overlapping with platform %d (expanded area)\n", i);
            return i;
        }
    }
    
    // Fallback: find closest platform within reasonable distance
    float closestDistance = 1000.0f;
    int closestPlatform = -1;
    
    for (int i = 0; i < game->arraySize; i++) {
        Rectangle platform = game->platforms[i];
        
        // Calculate distance between player center and platform center
        float playerCenterX = game->player.x + game->player.width / 2.0f;
        float playerCenterY = game->player.y + game->player.height / 2.0f;
        float platformCenterX = platform.x + platform.width / 2.0f;
        float platformCenterY = platform.y + platform.height / 2.0f;
        
        float dx = playerCenterX - platformCenterX;
        float dy = playerCenterY - platformCenterY;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance < closestDistance) {
            closestDistance = distance;
            closestPlatform = i;
        }
    }
    
    // Only return if within interaction range (1.5 times box size)
    if (closestDistance <= BOX_SIZE * 1.5f) {
        printf("Closest platform: %d (distance: %.1f)\n", closestPlatform, closestDistance);
        return closestPlatform;
    }
    
    printf("No platform within interaction range (closest: %.1f)\n", closestDistance);
    return -1;
}

// Bubble Sort specific data
typedef struct {
    int currentI;        // Current outer loop index (for tracking progress)
    int currentJ;        // Current inner loop index (for tracking progress)
    bool comparing;      // Whether currently in comparison state
    bool swapping;       // Whether currently performing a swap
    int comparisons;     // Total number of comparisons made
    int swaps;          // Total number of swaps performed
    int sortedUpTo;     // Index up to which array is sorted (for visual feedback)
} BubbleSortData;

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
    
    // Handle F key input for manual bubble sort interactions
    if (IsKeyPressed(KEY_F)) {
        int playerPlatform = GetPlayerPlatform(game);
        printf("F key pressed! Player platform: %d, Player pos: (%.1f, %.1f)\n", 
               playerPlatform, game->player.x, game->player.y);
        
        // Validate player is on a platform and within array bounds
        if (playerPlatform >= 0 && playerPlatform < game->arraySize) {
            if (!game->carrying && game->array[playerPlatform] != 0) {
                // Pick up number from box (matching reference code)
                game->playerNumber = game->array[playerPlatform];
                game->array[playerPlatform] = 0;
                game->carrying = true;
                data->comparisons++;
                printf("✓ Picked up number %d from platform %d\n", game->playerNumber, playerPlatform);
            } else if (game->carrying && game->array[playerPlatform] == 0) {
                // Place number into empty box (matching reference code)
                game->array[playerPlatform] = game->playerNumber;
                game->playerNumber = 0;
                game->carrying = false;
                printf("✓ Placed number %d on platform %d\n", game->array[playerPlatform], playerPlatform);
            } else if (game->carrying && game->array[playerPlatform] != 0) {
                // Swap with existing number (matching reference code)
                int temp = game->array[playerPlatform];
                game->array[playerPlatform] = game->playerNumber;
                game->playerNumber = temp;
                data->swaps++;
                printf("✓ Swapped: platform %d now has %d, carrying %d\n", 
                       playerPlatform, game->array[playerPlatform], game->playerNumber);
            } else {
                printf("✗ Cannot interact: carrying=%d, box_value=%d\n", game->carrying, game->array[playerPlatform]);
            }
        } else {
            printf("✗ Must be standing on a platform to interact (platform: %d)\n", playerPlatform);
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
        
        Vector2 textSize = MeasureTextEx(GetFontDefault(), numText, FONT_SIZE_BODY, 1);
        int textX = game->player.x + (game->player.width - textSize.x) / 2;
        int textY = game->player.y + (game->player.height - textSize.y) / 2;
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