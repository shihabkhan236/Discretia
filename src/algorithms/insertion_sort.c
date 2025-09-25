#include "insertion_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void InsertionSortInit(GameData *game)
{
    InsertionSortData *data = (InsertionSortData *)malloc(sizeof(InsertionSortData));
    if (!data)
    {
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
    data->canSkip = false;

    // Initialize comparison tracking
    data->leftIndex = -1;
    data->rightIndex = -1;
    data->originalLeft = 0;
    data->originalRight = 0;
    data->showComparison = false;

    game->algorithmData = data;
    printf("Insertion Sort initialized\n");
}

void InsertionSortUpdate(GameData *game)
{
    InsertionSortData *data = (InsertionSortData *)game->algorithmData;
    if (!data)
        return;

    // Set up comparison tracking for visual feedback - ONLY when starting to process a new element
    if (data->pickingElement && !data->showComparison && data->currentIndex < game->arraySize)
    {
        if (data->currentIndex > 0)
        {
            // Normal case: compare with previous element
            data->leftIndex = data->currentIndex - 1;
            data->rightIndex = data->currentIndex;
            data->originalLeft = game->array[data->leftIndex];
            data->originalRight = game->array[data->rightIndex];
            data->showComparison = true;

            // Check if current element can be skipped (already in correct position for insertion sort)
            // Element can be skipped if it's >= the previous element AND the sorted portion is valid
            bool inCorrectPosition = (data->originalRight >= data->originalLeft);
            
            // Additional check: ensure the sorted portion [0...currentIndex-1] is actually sorted
            bool sortedPortionValid = true;
            for (int i = 0; i < data->currentIndex - 1; i++)
            {
                if (game->array[i] > game->array[i + 1])
                {
                    sortedPortionValid = false;
                    break;
                }
            }
            
            data->canSkip = inCorrectPosition && sortedPortionValid;
        }
        else
        {
            // First element (index 0) is always in correct position
            data->canSkip = true;
            data->showComparison = false;
        }
    }

    // Handle G key for skipping when element is already in correct position
    if (IsKeyPressed(KEY_G))
    {
        int onSquare = GetPlayerPlatform(game);

        if (onSquare < 0)
        {
            printf("Not on any platform\n");
            return;
        }

        // Only allow skipping if we're at the current element and it's in correct position
        if (data->pickingElement && !game->carrying && onSquare == data->currentIndex && data->canSkip)
        {
            printf("✓ Skipping element %d at position %d - already in correct position relative to %d!\n",
                   data->originalRight, data->currentIndex, data->originalLeft);
            data->currentIndex++;

            // Reset comparison tracking for next element
            data->showComparison = false;
            data->leftIndex = -1;
            data->rightIndex = -1;
            data->originalLeft = 0;
            data->originalRight = 0;
            data->canSkip = false;

            // Check if insertion sort is complete
            if (data->currentIndex >= game->arraySize)
            {
                if (IsArraySorted(game->array, game->arraySize, true))
                {
                    ChangeState(STATE_LEVEL_COMPLETE);
                }
            }
        }
        else if (onSquare == data->currentIndex && !data->canSkip && data->pickingElement && !game->carrying)
        {
            printf("✗ Cannot skip! Element %d < %d - needs to be moved. Lost a heart!\n",
                   data->originalRight, data->originalLeft);
            game->hearts--;
            data->mistakes++;
            if (game->hearts <= 0)
            {
                ChangeState(STATE_GAME_OVER);
            }
        }
        else if (onSquare != data->currentIndex && data->pickingElement)
        {
            printf("✗ Can only skip when standing on current element (%d). You're on element %d\n",
                   data->currentIndex, onSquare);
        }
        else if (game->carrying)
        {
            printf("✗ Cannot skip while carrying an element\n");
        }
        else if (!data->pickingElement)
        {
            printf("✗ Can only skip when ready to pick up next element\n");
        }
        else
        {
            printf("✗ Cannot skip in current state\n");
        }
    }

    if (IsKeyPressed(KEY_F))
    {
        int onSquare = GetPlayerPlatform(game);
        printf("F pressed! On platform: %d, Carrying: %s, Current index: %d\n",
               onSquare, game->carrying ? "YES" : "NO", data->currentIndex);

        if (onSquare < 0)
        {
            printf("Not on any platform\n");
            return;
        }

        // STEP 1: Pick up elements - FIXED for proper insertion sort behavior
        if (!game->carrying && data->currentIndex < game->arraySize)
        {
            bool canPickUp = false;

            // In insertion sort, we should primarily pick up the current element being processed
            if (onSquare == data->currentIndex && game->array[onSquare] != 0)
            {
                canPickUp = true;
                printf("✓ Picking up current element %d from position %d\n",
                       game->array[onSquare], onSquare);
            }
            // Allow picking up from the already sorted section ONLY if we're in the middle of insertion
            // This happens when we placed an element but the section isn't fully sorted yet
            else if (onSquare < data->currentIndex && game->array[onSquare] != 0 && !data->pickingElement)
            {
                canPickUp = true;
                printf("✓ Picking up element %d from sorted section (fixing insertion)\n",
                       game->array[onSquare]);
            }

            if (canPickUp)
            {
                game->playerNumber = game->array[onSquare];
                game->array[onSquare] = 0; // Create empty space
                game->carrying = true;
                data->pickingElement = false;
                data->comparing = true;
                printf("✓ Picked up element %d from position %d\n", game->playerNumber, onSquare);
            }
            else
            {
                if (data->pickingElement)
                {
                    printf("✗ Should pick up the current element at position %d first\n", data->currentIndex);
                }
                else
                {
                    printf("✗ Can only pick up elements from positions being processed\n");
                }
            }
        }
        // STEP 2: Handle placement/swapping while carrying
        else if (game->carrying)
        {
            // Find where the empty space currently is
            int emptyIndex = -1;
            for (int i = 0; i < game->arraySize; i++)
            {
                if (game->array[i] == 0)
                {
                    emptyIndex = i;
                    break;
                }
            }

            if (emptyIndex == -1)
            {
                printf("Error: No empty space found!\n");
                return;
            }

            if (onSquare == emptyIndex)
            {
                // Place in empty space
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = 0;
                game->carrying = false;

                // Check if the processed section [0...currentIndex] is now correctly sorted
                // In insertion sort, after placing an element, we need to check if the section is in order
                bool processingComplete = true;

                // Check if the range [0...currentIndex] is sorted after placement
                for (int i = 0; i < data->currentIndex; i++)
                {
                    if (game->array[i] > game->array[i + 1])
                    {
                        processingComplete = false;
                        printf("Debug: Found out of order: array[%d]=%d > array[%d]=%d\n",
                               i, game->array[i], i + 1, game->array[i + 1]);
                        break;
                    }
                }

                if (processingComplete)
                {
                    // The section [0...currentIndex] is now sorted, move to next element
                    data->currentIndex++;
                    data->pickingElement = true;
                    data->comparing = false;

                    // Reset comparison tracking for next element
                    data->showComparison = false;
                    data->leftIndex = -1;
                    data->rightIndex = -1;
                    data->originalLeft = 0;
                    data->originalRight = 0;
                    data->canSkip = false;

                    printf("✓ Element inserted correctly! Section [0...%d] is sorted. Moving to element %d\n",
                           data->currentIndex - 1, data->currentIndex);
                }
                else
                {
                    // Section not fully sorted - we need to continue working on this section
                    // But don't reset to pickingElement=true, stay in comparing mode
                    data->comparing = true;
                    data->pickingElement = false;
                    printf("⚠ Section not fully sorted. Continue arranging elements in current section.\n");
                }
            }
            else if (abs(onSquare - emptyIndex) == 1 && game->array[onSquare] != 0)
            {
                // Adjacent swap - this moves the empty space
                bool validMove = false;

                if (emptyIndex > onSquare)
                {
                    // Moving empty space right (element moves left)
                    // In insertion sort, we can ONLY move left into the sorted portion
                    // Check if we're moving into the sorted portion AND maintaining order
                    if (onSquare < data->currentIndex && game->playerNumber <= game->array[onSquare])
                    {
                        validMove = true;
                    }
                    else if (onSquare >= data->currentIndex)
                    {
                        // Invalid: trying to move into unsorted portion
                        printf("✗ Invalid! In insertion sort, you can only move LEFT into the sorted portion\n");
                        game->hearts--;
                        data->mistakes++;
                        printf("Hearts remaining: %d\n", game->hearts);
                        if (game->hearts <= 0)
                        {
                            ChangeState(STATE_GAME_OVER);
                        }
                        return;
                    }
                    else
                    {
                        // Invalid: wrong order within sorted portion
                        printf("✗ Wrong order! Element %d cannot be placed before %d\n", game->playerNumber, game->array[onSquare]);
                        game->hearts--;
                        data->mistakes++;
                        printf("Hearts remaining: %d\n", game->hearts);
                        if (game->hearts <= 0)
                        {
                            ChangeState(STATE_GAME_OVER);
                        }
                        return;
                    }
                }
                else
                {
                    // Moving empty space left (element moves right)
                    // In insertion sort, this is NEVER allowed! Elements should only move left!
                    printf("✗ Invalid! In insertion sort, elements can ONLY move LEFT to find their insertion point\n");
                    printf("✗ You cannot move element %d to the right (toward unsorted portion)\n", game->playerNumber);
                    game->hearts--;
                    data->mistakes++;
                    printf("Hearts remaining: %d\n", game->hearts);
                    if (game->hearts <= 0)
                    {
                        ChangeState(STATE_GAME_OVER);
                    }
                    return; // Block this move completely
                }

                if (validMove)
                {
                    // Perform the swap
                    int temp = game->array[onSquare];
                    game->array[onSquare] = game->playerNumber;
                    game->playerNumber = temp;
                    data->shifts++;
                    data->comparisons++;
                    printf("✓ Swapped elements - moved %d left in sorted portion\n", game->array[onSquare]);
                }
            }
            else
            {
                printf("✗ Can only place in empty space or swap with adjacent elements\n");
            }
        }
    }

    // Check completion
    if (data->currentIndex >= game->arraySize)
    {
        bool allFilled = true;
        for (int i = 0; i < game->arraySize; i++)
        {
            if (game->array[i] == 0)
            {
                allFilled = false;
                break;
            }
        }

        if (allFilled && IsArraySorted(game->array, game->arraySize, true))
        {
            ChangeState(STATE_LEVEL_COMPLETE);
        }
    }
}

void InsertionSortGetStats(GameData *game, AlgorithmStats *stats)
{
    InsertionSortData *data = (InsertionSortData *)game->algorithmData;
    if (!data) return;

    // Primary stat: Comparisons, shifts, mistakes
    sprintf(stats->primaryStat, "Comparisons: %d | Shifts: %d | Mistakes: %d", 
            data->comparisons, data->shifts, data->mistakes);

    // Secondary stat and instructions based on algorithm state
    stats->hasInstruction = true;
    if (data->pickingElement && !game->carrying && data->currentIndex < game->arraySize)
    {
        sprintf(stats->secondaryStat, "Element at position %d: %d", 
                data->currentIndex, game->array[data->currentIndex]);

        if (data->canSkip)
        {
            strcpy(stats->instructionText, "Press G to SKIP (already in correct position) | Press F to pick up");
            stats->instructionColor = GAME_SORTED;
        }
        else
        {
            strcpy(stats->instructionText, "Press F to pick up the current element to insert");
            stats->instructionColor = GAME_COMPARING;
        }
    }
    else if (data->comparing && game->carrying)
    {
        strcpy(stats->secondaryStat, "Moving element to correct position");
        strcpy(stats->instructionText, "Press F on adjacent elements to swap, or empty box to place");
        stats->instructionColor = UI_TEXT_PRIMARY;
    }
    else if (game->carrying)
    {
        strcpy(stats->secondaryStat, "Placing element");
        strcpy(stats->instructionText, "Press F on an empty box to place the element");
        stats->instructionColor = UI_TEXT_PRIMARY;
    }
    else
    {
        strcpy(stats->secondaryStat, "");
        stats->hasInstruction = false;
    }

    strcpy(stats->goalText, "Goal: Sort numbers in ascending order (1, 2, 3, 4...)");
}

void InsertionSortRender(GameData *game)
{
    InsertionSortData *data = (InsertionSortData *)game->algorithmData;
    if (!data)
        return;

    // Only render visual highlights and game objects, no text UI

    // Simple visual indicators - only show completed sorted section
    for (int i = 0; i < data->currentIndex && i < game->arraySize; i++)
    {
        // Only highlight elements that are in their final correct positions
        // In insertion sort, elements in [0...currentIndex-1] that are properly ordered are sorted
        bool isInSortedSection = true;

        // Must not be empty and maintain ascending order
        if (game->array[i] == 0)
        {
            isInSortedSection = false;
        }
        else if (i > 0 && game->array[i - 1] > game->array[i])
        {
            isInSortedSection = false;
        }
        else if (i < data->currentIndex - 1 && game->array[i] > game->array[i + 1])
        {
            isInSortedSection = false;
        }

        // Only highlight if it's truly in the sorted section (green)
        if (isInSortedSection)
        {
            DrawRectangleLinesEx(game->platforms[i], 3, GAME_SORTED);
        }
    }

    // Highlight current player platform for upcoming moves only
    int playerPlatform = GetPlayerPlatform(game);
    if (playerPlatform >= 0)
    {
        if (data->pickingElement && playerPlatform == data->currentIndex)
        {
            DrawRectangleLinesEx(game->platforms[playerPlatform], 2, GAME_SELECTED);
        } 
    }

    // Show correct positions when H key is held
    if (IsKeyDown(KEY_H))
    {
        int target[MAX_ARRAY_SIZE];
        for (int i = 0; i < game->arraySize; i++)
            target[i] = i + 1;

        // Mark boxes that are already in final place
        for (int i = 0; i < game->arraySize; i++)
        {
            if (game->array[i] != 0 && game->array[i] == target[i])
            {
                DrawRectangleLinesEx(game->platforms[i], 2, GAME_SORTED);
            }
        }
    }
}

void InsertionSortCleanup(GameData *game)
{
    if (game && game->algorithmData)
    {
        free(game->algorithmData);
        game->algorithmData = NULL;

        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Insertion Sort cleaned up\n");
}

bool InsertionSortIsComplete(GameData *game)
{
    // Check if all boxes are filled (no empty boxes)
    for (int i = 0; i < game->arraySize; i++)
    {
        if (game->array[i] == 0)
        {
            return false;
        }
    }

    // Check if array is sorted in ascending order
    return IsArraySorted(game->array, game->arraySize, true);
}

void InsertionSortResetLevel(GameData *game, int level)
{
    // Reset algorithm-specific data
    InsertionSortData *data = (InsertionSortData *)game->algorithmData;
    if (data)
    {
        data->currentIndex = 1; // Start from second element
        data->insertionPosition = -1;
        data->pickingElement = true;
        data->comparing = false;
        data->shifting = false;
        data->inserting = false;
        data->comparisons = 0;
        data->shifts = 0;
        data->mistakes = 0;
        data->canSkip = false;

        // Reset comparison tracking
        data->leftIndex = -1;
        data->rightIndex = -1;
        data->originalLeft = 0;
        data->originalRight = 0;
        data->showComparison = false;
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

    printf("Insertion Sort level %d reset with %d elements\n", level, game->arraySize);
}