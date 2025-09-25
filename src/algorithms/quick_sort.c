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
#include <stdlib.h>

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
    data->manualElevation = false;
    data->pivotInSortedRegion = false;
    data->needsIncrementI = false;

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

    // Update platform positions for Quick Sort
    CalculateBoxPositions(game->platforms, game->arraySize);

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

    // Check if sorting is complete using proper QuickSort completion logic
    if (QuickSortIsComplete(game))
    {
        StartCompletionAnimation();
        return;
    }

    // Handle T key for manual elevation during pivot swapping
    if (IsKeyPressed(KEY_T))
    {
        if (data->needsIncrementI)
        {
            printf("First press F to increment i (move partition boundary), then T to elevate.\n");
        }
        else if (data->pivotSwapping && !data->manualElevation)
        {
            data->manualElevation = true;
            printf("✓ Rectangles elevated! Now you can swap the pivot.\n");
        }
        else if (data->pivotSwapping && data->manualElevation)
        {
            printf("Rectangles are already elevated.\n");
        }
        else
        {
            printf("T key only works during pivot swapping phase.\n");
        }
    }

    // Handle F key for swapping when needed
    if (IsKeyPressed(KEY_F))
    {
        printf("DEBUG: F key pressed - comparing=%s, pivotSwapping=%s, pivotInSortedRegion=%s, needsIncrementI=%s\n",
               data->comparing ? "true" : "false",
               data->pivotSwapping ? "true" : "false",
               data->pivotInSortedRegion ? "true" : "false",
               data->needsIncrementI ? "true" : "false");

        // Handle incrementing i before pivot swapping (Lomuto algorithm step)
        if (data->needsIncrementI)
        {
            // Increment i to create final pivot position
            data->i++;
            data->needsIncrementI = false;

            int finalPivotPos = data->i;
            printf("✓ i incremented to %d! This becomes the final pivot position.\n", finalPivotPos);

            // Now enter pivot swapping mode - user must press T to elevate before swapping
            data->pivotSwapping = true;
            data->manualElevation = false; // User must press T to elevate
            printf("🎯 NOW PRESS T to elevate rectangles, then swap arr[%d] with pivot[%d]\n",
                   finalPivotPos, data->pivotIndex);

            return;
        }

        if (!data->comparing && !data->pivotSwapping && !data->pivotInSortedRegion)
        {
            printf("Wait for the next comparison phase or pivot action\n");
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
            // Check if elevation is required first
            if (!data->manualElevation)
            {
                printf("Press T to elevate rectangles before swapping!\n");
                return;
            }

            int finalPivotPos = data->i; // i was already incremented

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
                        data->manualElevation = false;
                        data->hideOtherValues = false;
                        data->partitionComplete = true;
                        data->partitionsCompleted++;

                        // Mark this position as having a completed pivot
                        data->completedPivots[finalPivotPos] = true;

                        printf("✓ Pivot automatically falls into sorted region after swapping!\n");

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

        // Handle moving pivot to sorted region
        if (data->pivotInSortedRegion)
        {
            int finalPivotPos = data->i + 1; // For pivotInSortedRegion case, i wasn't incremented yet

            printf("DEBUG: Handling pivotInSortedRegion - finalPivotPos=%d, onSquare=%d, carrying=%s\n",
                   finalPivotPos, onSquare, game->carrying ? "true" : "false");

            // Check if player is on the pivot position
            if (onSquare != finalPivotPos)
            {
                printf("You must stand on position %d (pivot position) to move it to sorted region\n", finalPivotPos);
                return;
            }

            // Handle picking up/placing the pivot
            if (!game->carrying && game->array[onSquare] != 0)
            {
                // Pick up the pivot
                game->playerNumber = game->array[onSquare];
                game->array[onSquare] = 0;
                game->carrying = true;
                printf("Picked up pivot %d. Press F again to drop it in sorted region.\n", game->playerNumber);
            }
            else if (game->carrying && game->array[onSquare] == 0)
            {
                // Place the pivot back in sorted region
                game->array[onSquare] = game->playerNumber;
                game->playerNumber = 0;
                game->carrying = false;
                data->pivotInSortedRegion = false;
                data->partitionComplete = true;
                data->partitionsCompleted++;

                // Mark this position as having a completed pivot
                data->completedPivots[finalPivotPos] = true;

                printf("✓ Pivot placed in sorted region! Partition complete.\n");

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
                printf("DEBUG: Unexpected state - carrying=%s, array[%d]=%d\n",
                       game->carrying ? "true" : "false", onSquare, game->array[onSquare]);
            }
            return; // Exit after handling sorted region placement
        }

        // Handle regular comparison phase
        // In Lomuto: when arr[j] < pivot, increment i first, then player swaps manually

        // Check if swap is needed
        if (!data->needsSwap)
        {
            printf("Wrong! arr[j]=%d >= pivot=%d, no swap needed. Use G to skip. Lost a heart!\n",
                   data->originalJValue, data->originalPivotValue);
            game->hearts--;
            if (game->hearts <= 0)
            {
                ChangeState(STATE_GAME_OVER);
            }
            return;
        }

        // Check if we need to increment i first
        if (!data->waitingForSwap)
        {
            // Player must be on j position to start the swap process
            if (onSquare != data->j)
            {
                printf("You must stand on position %d (j) to start swap process\n", data->j);
                return;
            }

            // First step: increment i (move partition boundary)
            data->i++;

            printf("✓ i moved to %d! ", data->i);

            // Check if i and j are at the same position
            if (data->i == data->j)
            {
                // Automatic swap (animation-like) - no manual interaction needed
                printf("i and j are at the same position - automatic swap!\n");
                printf("✓ Automatic swap complete! arr[%d] stays in place\n", data->i);

                data->swaps++;
                data->waitingForSwap = false; // No need to wait for manual swap

                CompleteComparison(data, game);
                return;
            }
            else
            {
                // Different positions - manual swap required
                printf("Now manually swap arr[i]=%d with arr[j]=%d\n",
                       game->array[data->i], game->array[data->j]);
                data->waitingForSwap = true;

                return;
            }
        }

        // Second step: handle manual swapping between arr[i] and arr[j]
        int swapPos1 = data->i; // Position i (where smaller element should go)
        int swapPos2 = data->j; // Position j (current element being compared)

        // Check if player is on one of the swap positions
        if (onSquare != swapPos1 && onSquare != swapPos2)
        {
            printf("You must stand on position %d (i) or %d (j) to swap\n",
                   swapPos1, swapPos2);
            return;
        }

        // Handle the manual swapping process
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
                // Verify swap was done correctly: smaller element should be at i
                if (game->array[swapPos1] < game->array[data->pivotIndex])
                {
                    printf("✓ Correct swap! arr[i]=%d < pivot=%d\n",
                           game->array[swapPos1], game->array[data->pivotIndex]);
                    data->swaps++;
                    data->waitingForSwap = false; // Reset for next comparison

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

    // Handle G key for moving j forward (when no swap needed)
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

        // Player must be on j position to move j forward
        if (onSquare != data->j)
        {
            printf("You must stand on position %d (j) to move j forward\n", data->j);
            return;
        }

        if (data->needsSwap)
        {
            printf("Wrong! arr[j]=%d < pivot=%d, swap is needed. Use F key. Lost a heart!\n",
                   data->originalJValue, data->originalPivotValue);
            game->hearts--;
            if (game->hearts <= 0)
            {
                ChangeState(STATE_GAME_OVER);
            }
        }
        else
        {
            printf("✓ Correct! arr[j]=%d >= pivot=%d, moving j forward\n",
                   data->originalJValue, data->originalPivotValue);
            // Don't increment i when skipping, just move j forward
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
    if (data->needsIncrementI)
    {
        strcpy(stats->instructionText, "📈 Press F to increment i (move partition boundary) before pivot swap");
        stats->instructionColor = YELLOW;
    }
    else if (data->pivotInSortedRegion)
    {
        strcpy(stats->instructionText, "🔽 Pivot already in place! Press F to move to sorted region");
        stats->instructionColor = GAME_SORTED;
    }
    else if (data->pivotSwapping)
    {
        if (!data->manualElevation)
        {
            strcpy(stats->instructionText, "🔺 Press T to elevate rectangles for pivot swapping");
            stats->instructionColor = YELLOW;
        }
        else
        {
            strcpy(stats->instructionText, "🎯 PIVOT SWAPPING: Press F to swap (will auto-fall after)!");
            stats->instructionColor = YELLOW;
        }
    }
    else if (data->comparing)
    {
        if (data->needsSwap)
        {
            if (data->waitingForSwap)
            {
                if (game->carrying)
                {
                    strcpy(stats->instructionText, "SWAPPING: Press F to place/swap numbers between i and j");
                    stats->instructionColor = GAME_COMPARING;
                }
                else
                {
                    sprintf(stats->instructionText, "MANUAL SWAP: i=%d, j=%d - swap arr[i] with arr[j] (Press F)",
                            data->i, data->j);
                    stats->instructionColor = GAME_COMPARING;
                }
            }
            else
            {
                sprintf(stats->instructionText, "SWAP NEEDED: arr[j]=%d < pivot=%d - Press F to move i and swap",
                        data->originalJValue, data->originalPivotValue);
                stats->instructionColor = GAME_COMPARING;
            }
        }
        else
        {
            sprintf(stats->instructionText, "NO SWAP: arr[j]=%d >= pivot=%d - Press G to move j forward",
                    data->originalJValue, data->originalPivotValue);
            stats->instructionColor = GAME_SORTED;
        }
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
        // Highlight positions in Lomuto algorithm
        if (data->j < game->arraySize)
        {
            // Highlight j (current element being compared) in bright orange
            Color brightOrange = {255, 140, 0, 255}; // Bright orange for high visibility
            DrawRectangleLinesEx(game->platforms[data->j], 4, brightOrange);
        }

        // During manual swapping phase, highlight both i and j positions
        if (data->waitingForSwap && data->i >= 0 && data->i < game->arraySize)
        {
            Color brightGreen = {0, 255, 0, 255}; // Bright green for i position
            DrawRectangleLinesEx(game->platforms[data->i], 4, brightGreen);
        }
    }
    else if (data->pivotSwapping)
    {
        int finalPivotPos = data->i; // i was already incremented
        // Highlight the two positions that need to be swapped
        Color brightCyan = {0, 255, 255, 255};                                     // Bright cyan for final position
        Color brightMagenta = {255, 0, 255, 255};                                  // Bright magenta for current pivot
        DrawRectangleLinesEx(game->platforms[finalPivotPos], 4, brightCyan);       // Final position
        DrawRectangleLinesEx(game->platforms[data->pivotIndex], 4, brightMagenta); // Current pivot
    }
    else if (data->needsIncrementI)
    {
        // Highlight j position to indicate where user should press F to increment i
        Color brightYellow = {255, 255, 0, 255}; // Bright yellow for j position
        DrawRectangleLinesEx(game->platforms[data->j], 4, brightYellow);
    }

    // Fill pivot box with light red
    Color lightRed = {255, 150, 150, 128}; // Light red with some transparency
    DrawRectangleRec(game->platforms[data->pivotIndex], lightRed);

    // Draw i and j as rectangular bars instead of text labels
    for (int pos = 0; pos < game->arraySize; pos++)
    {
        Rectangle platform = game->platforms[pos];

        // Adjust platform position based on state
        if (QuickSortIsSwappingElement(game, pos))
        {
            platform.y -= BOX_SIZE + (BOX_SIZE / 2);
        }
        else if (QuickSortIsCompletedPivot(game, pos))
        {
            platform.y += BOX_SIZE + (BOX_SIZE / 2);
        }

        float baseY = platform.y + platform.height; // No gap - start right after platform
        float barWidth = platform.width;            // Same width as platform (full length)
        float barHeight = platform.width / 4;       // Height is 1/4 of platform width (bar-like)

        // Draw j bar (orange/yellow)
        if (data->j == pos)
        {
            Color jColor = {255, 165, 0, 255}; // Orange color for j
            Rectangle jBar = {
                platform.x, // Same x position as platform
                baseY,      // Directly attached to platform bottom
                barWidth,
                barHeight};
            DrawRectangleRec(jBar, jColor);
            DrawRectangleLinesEx(jBar, 2, BLACK); // Black outline

            // Draw "j" text on the bar
            int textWidth = MeasureText("j", 16);
            DrawText("j",
                     jBar.x + (jBar.width - textWidth) / 2,
                     jBar.y + (jBar.height - 16) / 2,
                     16, WHITE);
        }

        // Draw i bar (green) - same horizontal level as j bar
        if (data->i == pos && data->i >= 0)
        {
            Color iColor = {0, 180, 0, 255}; // Green color for i
            Rectangle iBar = {
                platform.x, // Same x position as platform
                baseY,      // Same level as j bar - directly attached to platform bottom
                barWidth,
                barHeight};
            DrawRectangleRec(iBar, iColor);
            DrawRectangleLinesEx(iBar, 2, BLACK); // Black outline

            // Draw "i" text on the bar
            int textWidth = MeasureText("i", 16);
            DrawText("i",
                     iBar.x + (iBar.width - textWidth) / 2,
                     iBar.y + (iBar.height - 16) / 2,
                     16, WHITE);
        }

        // Draw pivot indicator (keeping this as text since it's different)
        if (data->pivotIndex == pos)
        {
            Color pivotColor = {180, 50, 50, 255}; // Dark red
            DrawText("pivot", platform.x + (platform.width - MeasureText("pivot", FONT_SIZE_SMALL)) / 2,
                     baseY + barHeight + 5, FONT_SIZE_SMALL, pivotColor); // Below the bar level with small gap
        }
    }

    // Handle special case: "i" at position -1 (draw bar to the left of first platform)
    if (data->i == -1)
    {
        Rectangle firstPlatform = game->platforms[0];
        float barWidth = firstPlatform.width;      // Same width as platform
        float barHeight = firstPlatform.width / 4; // Height is 1/4 of platform width

        Color iColor = {0, 180, 0, 255}; // Green color for i
        Rectangle iBar = {
            firstPlatform.x - barWidth - 10,        // To the left of first platform
            firstPlatform.y + firstPlatform.height, // Same level as other bars - attached to platform bottom
            barWidth,
            barHeight};
        DrawRectangleRec(iBar, iColor);
        DrawRectangleLinesEx(iBar, 2, BLACK);

        int textWidth = MeasureText("i", 16);
        DrawText("i",
                 iBar.x + (iBar.width - textWidth) / 2,
                 iBar.y + (iBar.height - 16) / 2,
                 16, WHITE);
    }

    // Highlight current player platform if they can interact
    int playerPlatform = GetPlayerPlatform(game);
    if (playerPlatform >= 0)
    {
        bool canInteract = false;
        if (data->comparing)
        {
            // Player can interact if they're on j position (to start) or on i/j positions (during swap)
            if (data->waitingForSwap)
            {
                canInteract = (playerPlatform == data->i || playerPlatform == data->j);
            }
            else
            {
                canInteract = (playerPlatform == data->j);
            }
        }
        else if (data->needsIncrementI)
        {
            // Player can interact from j position to increment i
            canInteract = (playerPlatform == data->j);
        }
        else if (data->pivotSwapping)
        {
            int finalPivotPos = data->i; // i was already incremented
            canInteract = (playerPlatform == finalPivotPos || playerPlatform == data->pivotIndex);
        }
        else if (data->pivotInSortedRegion)
        {
            int finalPivotPos = data->i + 1; // For this case, i wasn't incremented yet
            canInteract = (playerPlatform == finalPivotPos);
        }

        if (canInteract)
        {
            DrawRectangleLinesEx(game->platforms[playerPlatform], 2, BLACK);
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
    if (!game || !game->algorithmData)
        return false;

    QuickSortData *data = (QuickSortData *)game->algorithmData;

    bool arrayIsSorted = IsArraySorted(game->array, game->arraySize, true);
    bool stackIsEmpty = (data->stackTop == -1);
    bool notInOperation = (!data->comparing && !data->pivotSwapping && !data->pivotInSortedRegion);

    // Debug output when array becomes sorted but algorithm isn't complete
    if (arrayIsSorted && (!stackIsEmpty || !notInOperation))
    {
        printf("DEBUG: Array is sorted but QuickSort not complete - stackTop=%d, comparing=%s, pivotSwapping=%s, pivotInSortedRegion=%s\n",
               data->stackTop, data->comparing ? "true" : "false",
               data->pivotSwapping ? "true" : "false", data->pivotInSortedRegion ? "true" : "false");
    }

    // QuickSort is complete when:
    // 1. The array is sorted AND
    // 2. No more partitions to process (stack is empty) AND
    // 3. Not in the middle of any operation
    return arrayIsSorted && stackIsEmpty && notInOperation;
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
        data->manualElevation = false;
        data->pivotInSortedRegion = false;
        data->needsIncrementI = false;

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
            // Enter pre-pivot-swap mode - user must first press F to increment i
            data->needsIncrementI = true;
            data->pivotSwapping = false;   // Not yet in swapping mode
            data->manualElevation = false; // Reset elevation state
            data->hideOtherValues = false; // Keep values visible
            data->comparing = false;
            printf("🎯 READY FOR PIVOT SWAP! First press F to increment i (i+1 becomes final pivot position)\n");
            printf("Current i=%d, will become i=%d after F key press\n", data->i, data->i + 1);
        }
        else
        {
            // Pivot is already in correct position, user must manually move it to sorted region
            data->partitionComplete = false;
            data->pivotInSortedRegion = true; // User must press F to move to sorted region
            data->pivotSwapping = false;      // Make sure we're not in swapping mode
            data->needsIncrementI = false;    // No need to increment i
            data->comparing = false;
            printf("✓ Pivot is already in correct position at %d! Press F to move it to sorted region.\n", finalPivotPos);
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
    if (!data->pivotSwapping || !data->manualElevation)
        return false;

    // During pivot swapping with manual elevation, elevate the pivot and its final position
    int finalPivotPos = data->i; // i was already incremented when we reach this state
    return (position == data->pivotIndex || position == finalPivotPos);
}