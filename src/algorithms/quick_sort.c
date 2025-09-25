/*
 * Lomuto Partition Algorithm Visualization:
 *
 * Initial state: [elements...] [pivot]
 * During partition: [<=pivot] [>pivot] [unsorted] [pivot]
 *                    ^          ^        ^         ^
 *                    |          |        |         |
 *                 sorted<    boundary  current   pivot
 *                           (partition)
 *
 * Player actions:
 * - If current element < pivot: Swap current with partition boundary (F key)
 * - If current element >= pivot: Skip to next element (G key)
 *
 * After each swap, partition boundary moves right.
 * At the end, pivot is swapped with partition boundary to final position.
 */

#include "quick_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include "../core/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper functions
static void PushStack(QuickSortData *data, int low, int high);
static bool PopStack(QuickSortData *data, int *low, int *high);
static void StartNewPartition(QuickSortData *data, GameData *game);
static void StartComparison(QuickSortData *data, GameData *game);
static void CompleteComparison(QuickSortData *data, GameData *game);

void QuickSortInit(GameData *game)
{
    QuickSortData *data = (QuickSortData *)malloc(sizeof(QuickSortData));
    if (!data)
    {
        printf("Error: Failed to allocate memory for QuickSortData\n");
        return;
    }

    // Initialize Lomuto partition variables
    data->low = 0;
    data->high = game->arraySize - 1;
    data->pivotIndex = data->high; // Pivot is always at the end
    data->i = data->low - 1;       // i starts at low-1 (conceptually -1 for first partition)
    data->j = data->low;           // j starts at low (index 0 for first partition)

    data->partitioning = true;
    data->comparing = false;
    data->needsSwap = false;
    data->partitionComplete = false;
    data->waitingForSwap = false;
    data->pivotSwapping = false;
    data->hideOtherValues = false;

    data->comparisons = 0;
    data->swaps = 0;
    data->partitionsCompleted = 0;

    data->originalJValue = 0;
    data->originalPivotValue = game->array[data->pivotIndex];
    data->originalIValue = 0;

    // Initialize stack for recursive calls
    data->stackTop = -1;

    // Initialize completed pivots tracking
    for (int k = 0; k < game->arraySize; k++)
    {
        data->completedPivots[k] = false;
    }

    game->algorithmData = data;

    // Update platform positions for Quick Sort with gaps
    CalculateQuickSortBoxPositions(game->platforms, game->arraySize, game);

    // Start first comparison if array has more than one element
    if (game->arraySize > 1)
    {
        StartComparison(data, game);
    }

    printf("Quick Sort (Traditional Lomuto) initialized: i=%d, j=%d, pivot=%d\n",
           data->i, data->j, data->originalPivotValue);
}

void QuickSortUpdate(GameData *game)
{
    QuickSortData *data = (QuickSortData *)game->algorithmData;
    if (!data)
        return;

    // Check if sorting is complete
    if (IsArraySorted(game->array, game->arraySize, true))
    {
        ChangeState(STATE_LEVEL_COMPLETE);
        return;
    }

    // Handle F key for swapping when needed
    if (IsKeyPressed(KEY_F))
    {
        if (!data->comparing && !data->pivotSwapping)
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

        // Handle pivot swapping phase
        if (data->pivotSwapping)
        {
            int finalPivotPos = data->i + 1;

            // Check if player is on one of the positions to swap
            if (onSquare != finalPivotPos && onSquare != data->pivotIndex)
            {
                printf("You must stand on position %d (final pivot position) or %d (current pivot) to swap\n",
                       finalPivotPos, data->pivotIndex);
                return;
            }

            // Handle the pivot swapping process
            if (!game->carrying && game->array[onSquare] != 0)
            {
                // Pick up number from current platform
                game->playerNumber = game->array[onSquare];
                game->array[onSquare] = 0;
                game->carrying = true;
                printf("Picked up %d from position %d\n", game->playerNumber, onSquare);
            }
            else if (game->carrying && game->array[onSquare] == 0)
            {
                // Place carried number on empty platform
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = 0;
                game->carrying = false;
                printf("Placed %d on position %d\n", game->array[onSquare], onSquare);

                // Check if pivot swap is complete
                if (game->array[finalPivotPos] != 0 && game->array[data->pivotIndex] != 0)
                {
                    // Verify correct pivot swap - the pivot value should now be at finalPivotPos
                    if (game->array[finalPivotPos] == data->originalPivotValue)
                    {
                        printf("🎉 PERFECT! Pivot swapped correctly to position %d!\n", finalPivotPos);
                        data->swaps++;
                        data->pivotSwapping = false;
                        data->hideOtherValues = false;
                        data->partitionComplete = true;
                        data->partitionsCompleted++;

                        // Mark this position as having a completed pivot
                        data->completedPivots[finalPivotPos] = true;

                        // Add sub-arrays to stack
                        if (finalPivotPos - 1 > data->low)
                        {
                            PushStack(data, data->low, finalPivotPos - 1);
                        }
                        if (finalPivotPos + 1 < data->high)
                        {
                            PushStack(data, finalPivotPos + 1, data->high);
                        }

                        // Start next partition
                        StartNewPartition(data, game);
                    }
                    else
                    {
                        printf("❌ Wrong swap! The pivot value (%d) should be at position %d. Continue rearranging.\n",
                               data->originalPivotValue, finalPivotPos);
                    }
                }
            }
            else if (game->carrying && game->array[onSquare] != 0)
            {
                // Swap carried number with number on platform
                int temp = game->array[onSquare];
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = temp;
                printf("Swapped numbers - now carrying %d\n", game->playerNumber);
            }
            return; // Exit after handling pivot swapping
        }

        // Handle regular comparison phase swapping
        // In traditional Lomuto: swap happens between arr[i+1] and arr[j]
        int swapPos1 = data->i + 1; // This is where smaller elements should go
        int swapPos2 = data->j;     // Current element being compared

        // Check if player is on one of the swap positions
        if (onSquare != swapPos1 && onSquare != swapPos2)
        {
            printf("You must stand on position %d (i+1=%d) or %d (j) to swap\n",
                   swapPos1, data->i + 1, swapPos2);
            return;
        }

        // Check if swap is actually needed
        if (!data->needsSwap)
        {
            printf("Wrong! arr[j]=%d >= pivot=%d, no swap needed. Lost a heart!\n",
                   data->originalJValue, data->originalPivotValue);
            game->hearts--;
            if (game->hearts <= 0)
            {
                ChangeState(STATE_GAME_OVER);
            }
            return;
        }

        // Handle the swapping process
        if (!game->carrying && game->array[onSquare] != 0)
        {
            // Pick up number from current platform
            game->playerNumber = game->array[onSquare];
            game->array[onSquare] = 0;
            game->carrying = true;
            printf("Picked up %d from position %d\n", game->playerNumber, onSquare);
        }
        else if (game->carrying && game->array[onSquare] == 0)
        {
            // Place carried number on empty platform
            game->array[onSquare] = game->playerNumber;
            game->playerNumber = 0;
            game->carrying = false;
            printf("Placed %d on position %d\n", game->array[onSquare], onSquare);

            // Check if swap is complete (both positions filled)
            if (game->array[swapPos1] != 0 && game->array[swapPos2] != 0)
            {
                // Verify swap was done correctly: smaller element should be at i+1
                if (game->array[swapPos1] < game->array[data->pivotIndex])
                {
                    printf("✓ Correct swap! arr[i+1]=%d < pivot=%d\n",
                           game->array[swapPos1], game->array[data->pivotIndex]);
                    data->swaps++;
                    data->i++; // Increment i after successful swap

                    // Update platform positions after partition boundary changes
                    CalculateQuickSortBoxPositions(game->platforms, game->arraySize, game);

                    CompleteComparison(data, game);
                }
                else
                {
                    printf("✗ Wrong swap! Continue rearranging to put smaller element at position %d\n", swapPos1);
                }
            }
        }
        else if (game->carrying && game->array[onSquare] != 0)
        {
            // Swap carried number with number on platform
            int temp = game->array[onSquare];
            game->array[onSquare] = game->playerNumber;
            game->playerNumber = temp;
            printf("Swapped numbers - now carrying %d\n", game->playerNumber);
        }
    }

    // Handle G key for skipping (when no swap needed)
    if (IsKeyPressed(KEY_G))
    {
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

        // Player must be on j position to skip
        if (onSquare != data->j)
        {
            printf("You must stand on position %d (j) to skip\n", data->j);
            return;
        }

        if (data->needsSwap)
        {
            printf("Wrong! arr[j]=%d < pivot=%d, swap is needed. Lost a heart!\n",
                   data->originalJValue, data->originalPivotValue);
            game->hearts--;
            if (game->hearts <= 0)
            {
                ChangeState(STATE_GAME_OVER);
            }
        }
        else
        {
            printf("✓ Correct! arr[j]=%d >= pivot=%d, no swap needed\n",
                   data->originalJValue, data->originalPivotValue);
            // Don't increment i when skipping
            CompleteComparison(data, game);
        }
    }
}

void QuickSortGetStats(GameData *game, AlgorithmStats *stats)
{
    QuickSortData *data = (QuickSortData *)game->algorithmData;
    if (!data)
        return;

    // Primary stat: Partitions, comparisons, swaps
    sprintf(stats->primaryStat, "Partitions: %d | Comparisons: %d | Swaps: %d",
            data->partitionsCompleted, data->comparisons, data->swaps);

    // Secondary stat: Current partition info
    sprintf(stats->secondaryStat, "Partition [%d...%d] | i=%d, j=%d | Pivot=%d (value=%d)",
            data->low, data->high, data->i, data->j, data->pivotIndex, game->array[data->pivotIndex]);

    // Instructions based on current state
    stats->hasInstruction = true;
    if (data->comparing)
    {
        if (data->needsSwap)
        {
            if (game->carrying)
            {
                strcpy(stats->instructionText, "SWAPPING: Press F to place/swap numbers");
                stats->instructionColor = GAME_COMPARING;
            }
            else
            {
                sprintf(stats->instructionText, "SWAPPING NEEDED: arr[j]=%d < pivot=%d - Press F to pick up",
                        data->originalJValue, data->originalPivotValue);
                stats->instructionColor = GAME_COMPARING;
            }
        }
        else
        {
            strcpy(stats->instructionText, "Press G to SKIP - arr[j] >= pivot, no swap needed");
            stats->instructionColor = GAME_SORTED;
        }
    }
    else if (data->pivotSwapping)
    {
        strcpy(stats->instructionText, "🎯 PIVOT SWAPPING: Press F to swap pivot to final position!");
        stats->instructionColor = YELLOW;
    }
    else
    {
        strcpy(stats->instructionText, "Processing next comparison...");
        stats->instructionColor = UI_TEXT_PRIMARY;
    }

    strcpy(stats->goalText, "Goal: Partition using Lomuto algorithm (i,j pointers)");
}

void QuickSortRender(GameData *game)
{
    QuickSortData *data = (QuickSortData *)game->algorithmData;
    if (!data)
        return;

    // Only render visual highlights and game objects, no text UI

    if (data->comparing)
    {
        // Highlight positions in traditional Lomuto algorithm
        if (data->j < game->arraySize)
        {
            // Highlight j (current element being compared) in bright orange
            Color brightOrange = {255, 140, 0, 255}; // Bright orange for high visibility
            DrawRectangleLinesEx(game->platforms[data->j], 4, brightOrange);
        }

        if (data->needsSwap && data->i + 1 < game->arraySize)
        {
            // Highlight i+1 (where smaller element should go) in bright cyan
            Color brightCyan = {0, 255, 255, 255}; // Bright cyan for high visibility
            DrawRectangleLinesEx(game->platforms[data->i + 1], 4, brightCyan);
        }
    }
    else if (data->pivotSwapping)
    {
        int finalPivotPos = data->i + 1;
        // Highlight the two positions that need to be swapped
        Color brightCyan = {0, 255, 255, 255};                                     // Bright cyan for final position
        Color brightMagenta = {255, 0, 255, 255};                                  // Bright magenta for current pivot
        DrawRectangleLinesEx(game->platforms[finalPivotPos], 4, brightCyan);       // Final position
        DrawRectangleLinesEx(game->platforms[data->pivotIndex], 4, brightMagenta); // Current pivot
    }

    // Fill pivot box with light red
    Color lightRed = {255, 150, 150, 128}; // Light red with some transparency
    DrawRectangleRec(game->platforms[data->pivotIndex], lightRed);

    // Highlight i boundary (elements <= i are smaller than pivot)
    for (int idx = data->low; idx <= data->i && idx < game->arraySize; idx++)
    {
        DrawRectangleLinesEx(game->platforms[idx], 2, DARKGREEN);
    }

    // Highlight current player platform if they can interact
    int playerPlatform = GetPlayerPlatform(game);
    if (playerPlatform >= 0)
    {
        bool canInteract = false;
        if (data->comparing)
        {
            canInteract = (playerPlatform == data->j) ||
                          (data->needsSwap && playerPlatform == data->i + 1);
        }
        else if (data->pivotSwapping)
        {
            int finalPivotPos = data->i + 1;
            canInteract = (playerPlatform == finalPivotPos || playerPlatform == data->pivotIndex);
        }

        if (canInteract)
        {
            DrawRectangleLinesEx(game->platforms[playerPlatform], 2, WHITE);
        }
    }
}

void QuickSortCleanup(GameData *game)
{
    if (game && game->algorithmData)
    {
        free(game->algorithmData);
        game->algorithmData = NULL;

        // Reset player state
        game->carrying = false;
        game->playerNumber = 0;
    }
    printf("Quick Sort cleaned up\n");
}

bool QuickSortIsComplete(GameData *game)
{
    return IsArraySorted(game->array, game->arraySize, true);
}

void QuickSortResetLevel(GameData *game, int level)
{
    // Reset algorithm-specific data
    QuickSortData *data = (QuickSortData *)game->algorithmData;
    if (data)
    {
        data->low = 0;
        data->high = game->arraySize - 1;
        data->pivotIndex = data->high; // Pivot at end
        data->i = data->low - 1;       // i starts at low-1
        data->j = data->low;           // j starts at low

        data->partitioning = true;
        data->comparing = false;
        data->needsSwap = false;
        data->partitionComplete = false;
        data->waitingForSwap = false;
        data->pivotSwapping = false;
        data->hideOtherValues = false;

        data->comparisons = 0;
        data->swaps = 0;
        data->partitionsCompleted = 0;

        data->originalJValue = 0;
        data->originalPivotValue = 0;
        data->originalIValue = 0;
        data->stackTop = -1;

        // Reset completed pivots tracking
        for (int k = 0; k < game->arraySize; k++)
        {
            data->completedPivots[k] = false;
        }
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

    // Re-initialize data for new level
    if (data && game->arraySize > 1)
    {
        data->high = game->arraySize - 1;
        data->pivotIndex = data->high;
        data->originalPivotValue = game->array[data->pivotIndex];
        StartComparison(data, game);
    }

    printf("Quick Sort level %d reset: Array size=%d, i=%d, j=%d, pivot=%d\n",
           level, game->arraySize, data ? data->i : -1, data ? data->j : 0,
           game->array[game->arraySize - 1]);
}

// Helper function implementations
static void PushStack(QuickSortData *data, int low, int high)
{
    if (data->stackTop < MAX_ARRAY_SIZE - 2)
    {
        data->stackTop++;
        data->stackLow[data->stackTop] = low;
        data->stackHigh[data->stackTop] = high;
        printf("Pushed partition [%d...%d] to stack\n", low, high);
    }
}

static bool PopStack(QuickSortData *data, int *low, int *high)
{
    if (data->stackTop >= 0)
    {
        *low = data->stackLow[data->stackTop];
        *high = data->stackHigh[data->stackTop];
        data->stackTop--;
        printf("Popped partition [%d...%d] from stack\n", *low, *high);
        return true;
    }
    return false;
}

static void StartNewPartition(QuickSortData *data, GameData *game)
{
    int newLow, newHigh;
    if (PopStack(data, &newLow, &newHigh))
    {
        data->low = newLow;
        data->high = newHigh;
        data->pivotIndex = newHigh; // Pivot always at end
        data->i = newLow - 1;       // i starts at low-1
        data->j = newLow;           // j starts at low
        data->partitionComplete = false;
        data->originalPivotValue = game->array[data->pivotIndex];

        printf("Starting new partition [%d...%d]: i=%d, j=%d, pivot=%d\n",
               data->low, data->high, data->i, data->j, data->originalPivotValue);

        // Update platform positions for new partition
        CalculateQuickSortBoxPositions(game->platforms, game->arraySize, game);

        StartComparison(data, game);
    }
    else
    {
        printf("No more partitions - Quick Sort complete!\n");
        data->partitioning = false;

        // Reset to regular positioning when sorting is complete
        CalculateBoxPositions(game->platforms, game->arraySize);
    }
}

static void StartComparison(QuickSortData *data, GameData *game)
{
    if (data->j < data->high)
    { // Don't compare with pivot itself
        data->comparing = true;
        data->originalJValue = game->array[data->j];
        data->originalPivotValue = game->array[data->pivotIndex];
        data->needsSwap = (data->originalJValue < data->originalPivotValue);

        printf("Comparing arr[j=%d]=%d with pivot[%d]=%d -> %s\n",
               data->j, data->originalJValue, data->pivotIndex, data->originalPivotValue,
               data->needsSwap ? "SWAP NEEDED" : "SKIP");
    }
}

static void CompleteComparison(QuickSortData *data, GameData *game)
{
    data->comparisons++;
    data->comparing = false;
    data->j++; // Always increment j after each comparison

    // Check if we've reached the pivot (j == high)
    if (data->j >= data->high)
    {
        // Start interactive pivot swapping phase
        int finalPivotPos = data->i + 1;
        if (finalPivotPos != data->pivotIndex)
        {
            // Enter pivot swapping mode - user must swap manually
            data->pivotSwapping = true;
            data->hideOtherValues = false; // Keep values visible during pivot swapping
            data->comparing = false;
            printf("🎯 TIME TO SWAP PIVOT! Swap arr[i+1=%d] with pivot[%d]\n", finalPivotPos, data->pivotIndex);
            printf("Values remain visible during pivot swap!\n");
        }
        else
        {
            // Pivot is already in correct position, proceed directly
            data->partitionComplete = true;
            data->partitionsCompleted++;

            // Mark this position as having a completed pivot
            data->completedPivots[finalPivotPos] = true;

            // Add sub-arrays to stack
            if (finalPivotPos - 1 > data->low)
            {
                PushStack(data, data->low, finalPivotPos - 1);
            }
            if (finalPivotPos + 1 < data->high)
            {
                PushStack(data, finalPivotPos + 1, data->high);
            }

            // Start next partition
            StartNewPartition(data, game);
        }
    }
    else
    {
        // Continue with next comparison
        StartComparison(data, game);
    }
}

// Utility function for UI rendering - returns true if value at position should be hidden
bool QuickSortShouldHideValue(GameData *game, int position)
{
    // Suppress unused parameter warnings
    (void)game;
    (void)position;

    // Values are no longer hidden during pivot swapping for better user experience
    return false;
}

bool QuickSortIsCompletedPivot(GameData *game, int position)
{
    if (!game || !game->algorithmData)
        return false;

    QuickSortData *data = (QuickSortData *)game->algorithmData;
    return data->completedPivots[position];
}

// Check if an element is involved in pivot swapping (should be elevated)
bool QuickSortIsSwappingElement(GameData *game, int position)
{
    if (!game || !game->algorithmData)
        return false;

    QuickSortData *data = (QuickSortData *)game->algorithmData;
    if (!data->pivotSwapping)
        return false;

    // During pivot swapping, elevate the pivot and its final position
    int finalPivotPos = data->i + 1;
    return (position == data->pivotIndex || position == finalPivotPos);
}