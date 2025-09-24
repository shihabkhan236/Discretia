#include "bubble_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void BubbleSortInit(GameData *game)
{
    BubbleSortData *data = (BubbleSortData *)malloc(sizeof(BubbleSortData));
    if (!data)
    {
        printf("Error: Failed to allocate memory for BubbleSortData\n");
        return;
    }

    data->currentI = 0;
    data->currentJ = 0;
    data->comparing = false;
    data->swapping = false;
    data->comparisons = 0;
    data->swaps = 0;
    data->originalLeft = 0;
    data->originalRight = 0;

    game->algorithmData = data;
    printf("Bubble Sort initialized\n");
}

void BubbleSortUpdate(GameData *game)
{
    BubbleSortData *data = (BubbleSortData *)game->algorithmData;
    if (!data)
        return;

    // Auto-advance bubble sort algorithm state
    if (!data->comparing && !data->swapping)
    {
        // Check if current pass is complete
        if (data->currentJ >= game->arraySize - 1 - data->currentI)
        {
            data->currentI++;
            data->currentJ = 0;

            // Check if bubble sort is complete
            if (data->currentI >= game->arraySize - 1)
            {
                ChangeState(STATE_LEVEL_COMPLETE);
                return;
            }
            printf("Pass %d complete, starting pass %d\n", data->currentI - 1, data->currentI);
        }
        else
        {
            data->comparing = true;
            // Store original values for comparison (before any pickup/manipulation)
            data->originalLeft = game->array[data->currentJ];
            data->originalRight = game->array[data->currentJ + 1];
            printf("Comparing positions %d and %d: %d vs %d\n",
                   data->currentJ, data->currentJ + 1,
                   data->originalLeft, data->originalRight);
        }
    }

    // Handle interaction key input for guided bubble sort
    // Using F key for pickup/place/swap, G key for skipping
    if (IsKeyPressed(KEY_F))
    {
        // Only allow interaction if we're in comparing state
        if (!data->comparing)
        {
            printf("Wait for the next comparison phase\n");
            return;
        }

        int onSquare = GetPlayerPlatform(game); /* platform player stands on */
        if (onSquare < 0)
        { /* not on any box → ignore */
            printf("Not on a platform – nothing happens\n");
            return;
        }

        // Check if player is on one of the positions being compared
        if (onSquare != data->currentJ && onSquare != data->currentJ + 1)
        {
            printf("You must stand on position %d or %d to interact\n", data->currentJ, data->currentJ + 1);
            return;
        }

        // Check if swap is needed first (use original values, not current array state)
        bool needsSwap = data->originalLeft > data->originalRight;
        if (!needsSwap)
        {
            // Wrong: F pressed but no swap needed
            printf("Wrong! These numbers are already in correct order (%d <= %d). Lost a heart!\n",
                   data->originalLeft, data->originalRight);
            game->hearts--;
            if (game->hearts <= 0)
                ChangeState(STATE_GAME_OVER);

            // Stay on the same comparison - don't advance currentJ
            data->comparing = true;
            return;
        }

        // Gamified swapping mechanics
        if (!game->carrying && game->array[onSquare] != 0)
        {
            // Pick up number from current platform
            game->playerNumber = game->array[onSquare];
            game->array[onSquare] = 0; // Make platform empty
            game->carrying = true;
            printf("Picked up %d from platform %d\n", game->playerNumber, onSquare);
            return;
        }
        else if (game->carrying && game->array[onSquare] == 0)
        {
            // Place carried number on empty platform
            game->array[onSquare] = game->playerNumber;
            game->playerNumber = 0;
            game->carrying = false;
            printf("Placed %d on platform %d\n", game->array[onSquare], onSquare);

            // Check if swap is complete (no empty spaces in comparison area)
            if (game->array[data->currentJ] != 0 && game->array[data->currentJ + 1] != 0)
            {
                // Verify that the swap actually resulted in correct ordering
                // For a valid bubble sort swap, left should be <= right after completion
                if (game->array[data->currentJ] <= game->array[data->currentJ + 1])
                {
                    printf("Swap complete and correctly ordered!\n");
                    data->swaps++;
                    data->comparisons++;
                    data->comparing = false;
                    data->currentJ++;
                }
                else
                {
                    // Values are back but still in wrong order - swap not actually completed
                    printf("Numbers are back but still in wrong order! Continue swapping.\n");
                }
            }
            return;
        }
        else if (game->carrying && game->array[onSquare] != 0)
        {
            // Swap carried number with number on platform (only if adjacent to empty space)
            int emptyIndex = -1;
            for (int i = 0; i < game->arraySize; ++i)
            {
                if (game->array[i] == 0)
                {
                    emptyIndex = i;
                    break;
                }
            }

            if (emptyIndex == -1)
            {
                printf("No empty space to complete swap\n");
                return;
            }

            // Only allow adjacent swaps within the comparison area
            if ((onSquare == emptyIndex - 1 || onSquare == emptyIndex + 1) &&
                (onSquare == data->currentJ || onSquare == data->currentJ + 1) &&
                (emptyIndex == data->currentJ || emptyIndex == data->currentJ + 1))
            {
                int temp = game->array[onSquare];
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = temp;
                printf("Swapped numbers - now carrying %d\n", game->playerNumber);
            }
            else
            {
                printf("Can only swap within the comparison area\n");
            }
            return;
        }
    }

    // Handle skip (G key) - for when no swap is needed
    if (IsKeyPressed(KEY_G))
    {
        // Only allow interaction if we're in comparing state
        if (!data->comparing)
        {
            printf("Wait for the next comparison phase\n");
            return;
        }

        int onSquare = GetPlayerPlatform(game);
        if (onSquare < 0)
        {
            printf("Not on a platform – nothing happens\n");
            return;
        }

        // Check if player is on one of the positions being compared
        if (onSquare != data->currentJ && onSquare != data->currentJ + 1)
        {
            printf("You must stand on position %d or %d to interact\n", data->currentJ, data->currentJ + 1);
            return;
        }

        // Use original values for comparison logic
        bool needsSwap = data->originalLeft > data->originalRight;

        if (!needsSwap)
        {
            // Correct: G pressed when no swap needed
            printf("Correct! No swap needed - already in order (%d <= %d)\n",
                   data->originalLeft, data->originalRight);
            data->comparisons++;
            data->comparing = false;
            data->currentJ++;
        }
        else
        {
            // Wrong: G pressed but swap was needed
            printf("Wrong! These numbers need to be swapped (%d > %d). Lost a heart!\n",
                   data->originalLeft, data->originalRight);
            game->hearts--;
            if (game->hearts <= 0)
                ChangeState(STATE_GAME_OVER);

            // Stay on the same comparison
            data->comparing = true;
        }
        return;
    }

    // Check for completion - all elements must be in ascending order
    if (IsArraySorted(game->array, game->arraySize, true))
    {
        ChangeState(STATE_LEVEL_COMPLETE);
    }
}

void BubbleSortRender(GameData *game)
{
    BubbleSortData *data = (BubbleSortData *)game->algorithmData;
    if (!data)
        return;

    // Draw algorithm-specific UI
    char statsText[128];
    sprintf(statsText, "Pass: %d | Comparisons: %d | Swaps: %d", data->currentI + 1, data->comparisons, data->swaps);
    DrawText(statsText, 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);

    // Show current comparison state
    if (data->comparing)
    {
        char compText[128];
        sprintf(compText, "Comparing %d and %d", data->originalLeft, data->originalRight);
        DrawText(compText, 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);

        // Show instructions based on player state and what's needed (use original values)
        bool needsSwap = data->originalLeft > data->originalRight;

        if (needsSwap)
        {
            if (game->carrying)
            {
                DrawText("SWAPPING: Press F to place/swap numbers", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
                DrawText("Complete the swap by moving numbers around", 20, 180, FONT_SIZE_SMALL, GAME_COMPARING);
            }
            else
            {
                DrawText("SWAPPING NEEDED: Press F to pick up a number", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
                DrawText("Left number is larger - start swapping!", 20, 180, FONT_SIZE_SMALL, GAME_COMPARING);
            }
        }
        else
        {
            DrawText("Press G to SKIP - numbers already in correct order", 20, 160, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
            DrawText("No swap needed here!", 20, 180, FONT_SIZE_SMALL, GAME_SORTED);
        }

        // Highlight the positions being compared
        if (data->currentJ < game->arraySize && data->currentJ + 1 < game->arraySize)
        {
            DrawRectangleLinesEx(game->platforms[data->currentJ], 3, BUBBLE_COMPARE);
            DrawRectangleLinesEx(game->platforms[data->currentJ + 1], 3, BUBBLE_COMPARE);
        }
    }
    else
    {
        DrawText("Moving to next comparison...", 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    }

    DrawText("Goal: Sort numbers in ascending order using bubble sort", 20, 200, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);

    // Highlight sorted portion (elements that have "bubbled" to their final position)
    for (int i = game->arraySize - data->currentI; i < game->arraySize; i++)
    {
        DrawRectangleLinesEx(game->platforms[i], 2, GAME_SORTED);
    }

    // Highlight current player platform if they can interact
    int playerPlatform = GetPlayerPlatform(game);
    if (playerPlatform >= 0 && data->comparing)
    {
        if (playerPlatform == data->currentJ || playerPlatform == data->currentJ + 1)
        {
            DrawRectangleLinesEx(game->platforms[playerPlatform], 2, GAME_HIGHLIGHT);
        }
    }
}

void BubbleSortCleanup(GameData *game)
{
    if (game && game->algorithmData)
    {
        free(game->algorithmData);
        game->algorithmData = NULL;

        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Bubble Sort cleaned up\n");
}

bool BubbleSortIsComplete(GameData *game)
{
    BubbleSortData *data = (BubbleSortData *)game->algorithmData;
    if (!data)
        return false;

    // Bubble sort is complete when we've done n-1 passes
    return (data->currentI >= game->arraySize - 1) && IsArraySorted(game->array, game->arraySize, true);
}

void BubbleSortResetLevel(GameData *game, int level)
{
    // Reset algorithm-specific data
    BubbleSortData *data = (BubbleSortData *)game->algorithmData;
    if (data)
    {
        data->currentI = 0;
        data->currentJ = 0;
        data->comparing = false;
        data->swapping = false;
        data->comparisons = 0;
        data->swaps = 0;
        data->originalLeft = 0;
        data->originalRight = 0;
    }

    // Reset player carrying state
    game->carrying = false;
    game->playerNumber = 0;

    // Generate level array
    switch (level)
    {
    case 0: // Tutorial - simple 4-element array
        game->arraySize = 4;
        int tutorial[] = {4, 2, 3, 1};
        for (int i = 0; i < game->arraySize; i++)
        {
            game->array[i] = tutorial[i];
        }
        break;
    case 1: // Easy - 5 elements
        game->arraySize = 5;
        for (int i = 0; i < game->arraySize; i++)
        {
            game->array[i] = i + 1;
        }
        ShuffleArray(game->array, game->arraySize);
        break;
    default: // Progressive difficulty
        game->arraySize = 4 + level;
        if (game->arraySize > MAX_ARRAY_SIZE)
            game->arraySize = MAX_ARRAY_SIZE;

        // Generate sequential numbers and shuffle
        for (int i = 0; i < game->arraySize; i++)
        {
            game->array[i] = i + 1;
        }
        ShuffleArray(game->array, game->arraySize);
        break;
    }

    printf("Bubble Sort level %d reset with %d elements\n", level, game->arraySize);
}