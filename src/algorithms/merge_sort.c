#include "merge_sort.h"
#include "algorithm.h"
#include "../core/game.h"
#include "../core/player.h"
#include "../utils/colors.h"
#include "../ui/ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

// Subarray structure for merge sort tree
typedef struct {
    int* elements;           // Array elements
    int size;               // Number of elements
    Vector2 position;       // Screen position
    Rectangle* platforms;   // Platform rectangles
    int level;             // Level in tree (0=base, 1=first split, etc.)
    int index;             // Index within level
    int parentIndex;       // Index of parent subarray (-1 for level 1, which uses original array)
    bool isActive;         // Whether subarray is visible
    bool canSplit;         // Whether G key works on this array
    bool isExhausted;      // Whether all elements merged out
    bool isLeftChild;      // Whether this is a left child of its parent
} Subarray;

// Merge sort game data
typedef struct {
    Subarray* subarrays;    // Dynamic array of subarrays (additional arrays above original)
    int subarrayCount;      // Number of active subarrays
    int subarrayCapacity;   // Allocated capacity
    
    // Game state
    enum { PHASE_SPLITTING, PHASE_MERGING, PHASE_COMPLETE } currentPhase;
    int maxLevel;          // Highest level reached
    bool recursionComplete; // Whether all splitting is done and merging can begin
    
    // Merge state
    int leftMergeIndex;    // Left subarray being merged
    int rightMergeIndex;   // Right subarray being merged
    int targetMergeIndex;  // Target for merge result
    int leftPointer;       // Position in left subarray
    int rightPointer;      // Position in right subarray
    
    // Selection state
    bool highlightingEnabled; // Whether to show merge highlights
    int highlightedElement;   // Which element is highlighted (-1 = none)
    int highlightedSubarray;  // Which subarray contains highlighted element
    
    // Comparison highlighting (show both elements being compared)
    int leftCompareElement;   // Left element being compared (-1 = none)
    int rightCompareElement;  // Right element being compared (-1 = none)
    
    // Track which elements are being worked on
    bool* originalActive;  // Which elements in original array are still active
    
    // Player carrying system
    int playerCarrying;    // Number player is carrying (0 = nothing)
    bool hasNumber;        // Whether player is carrying a number
    
    // Focus system for sequential recursion
    int currentFocusIndex; // Index of currently focused subarray (-1 = original array)
    int* focusStack;       // Stack of focus indices for recursion tracking
    int focusStackSize;    // Current stack size
    int focusStackCapacity; // Allocated capacity for focus stack
    

    
    // Statistics
    int splitCount;
    int mergeCount;
    
    // Camera integration
    Vector2 cameraTarget;
    bool cameraFollowPlayer;
} MergeSortData;

// Forward declarations
static void AddSubarray(MergeSortData* data, Subarray subarray);
static void UpdateAllPlatforms(MergeSortData* data, GameData* game);
static Vector2 CalculateSubarrayPosition(MergeSortData* data, GameData* game, int level, int parentIndex, bool isLeftChild, int elementsInSubarray);
static void SplitArray(MergeSortData* data, int subarrayIndex, GameData* game);
static void UpdateMergeSortCamera(GameData* game, MergeSortData* data);
static void StartMergePhase(MergeSortData* data);
static void UpdateMergeHighlights(MergeSortData* data);
static bool ProcessMergeChoice(GameData* game, MergeSortData* data, int subarrayIndex, int elementIndex);
static void FindMergeablePairs(MergeSortData* data);
static bool CanMerge(MergeSortData* data, int leftIndex, int rightIndex);
static void InitializeFocusSystem(MergeSortData* data);
static void UpdateFocus(MergeSortData* data);
static void UpdateSubarrayFocusStates(MergeSortData* data);
static void HandleMergeCompletion(MergeSortData* data, GameData* game);
static int FindNextFocusAfterMerge(MergeSortData* data, int mergedParentIndex);
static void PrintMergeSortState(MergeSortData* data);
static void MoveFocusAfterSplit(MergeSortData* data, int leftChildIndex, int rightChildIndex);
static bool IsFocused(MergeSortData* data, int subarrayIndex);
static int FindSibling(MergeSortData* data, int subarrayIndex);
static void PushFocusStack(MergeSortData* data, int focusIndex);
static int PopFocusStack(MergeSortData* data);

void MergeSortInit(GameData* game) {
    // Ensure any previous algorithm data is cleaned up
    if (game->algorithmData) {
        printf("Warning: Previous algorithm data exists, cleaning up\n");
        free(game->algorithmData);
        game->algorithmData = NULL;
    }
    
    MergeSortData* data = (MergeSortData*)malloc(sizeof(MergeSortData));
    if (!data) {
        printf("Error: Failed to allocate memory for MergeSortData\n");
        return;
    }
    
    // Initialize data structure with proper error checking
    data->subarrayCapacity = 32; // Start with reasonable capacity
    data->subarrays = (Subarray*)malloc(data->subarrayCapacity * sizeof(Subarray));
    if (!data->subarrays) {
        printf("Error: Failed to allocate memory for subarrays\n");
        free(data);
        return;
    }
    
    data->subarrayCount = 0;
    data->currentPhase = PHASE_SPLITTING;
    data->maxLevel = 0;
    data->recursionComplete = false;
    
    // Initialize merge state
    data->leftMergeIndex = -1;
    data->rightMergeIndex = -1;
    data->targetMergeIndex = -1;
    data->leftPointer = 0;
    data->rightPointer = 0;
    
    // Initialize selection state
    data->highlightingEnabled = false;
    data->highlightedElement = -1;
    data->highlightedSubarray = -1;
    data->leftCompareElement = -1;
    data->rightCompareElement = -1;
    
    // Initialize player carrying system
    data->playerCarrying = 0;
    data->hasNumber = false;
    
    // Initialize focus system
    InitializeFocusSystem(data);
    

    
    // Track which original elements are active
    data->originalActive = (bool*)malloc(MAX_ARRAY_SIZE * sizeof(bool));
    if (!data->originalActive) {
        printf("Error: Failed to allocate memory for originalActive\n");
        free(data->subarrays);
        free(data);
        return;
    }
    
    // Initialize statistics
    data->splitCount = 0;
    data->mergeCount = 0;
    
    // Initialize camera integration - work with existing camera system
    Vector2 playerCenter = { 
        game->player.x + game->player.width/2, 
        game->player.y + game->player.height/2 
    };
    data->cameraTarget = playerCenter;
    data->cameraFollowPlayer = true;
    
    // Set camera to follow player initially
    game->camera.target = playerCenter;
    
    game->algorithmData = data;
    printf("Merge Sort initialized - will work with original array\n");
}

// Function to handle subarray collision detection - called from player.c
int MergeSortGetPlayerPlatform(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return -1;
    
    // Check collision with subarray platforms using same logic as GetPlayerPlatform
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (!sub->isActive) continue;
        
        for (int j = 0; j < sub->size; j++) {
            Rectangle platform = sub->platforms[j];
            
            // Use exact same collision logic as GetPlayerPlatform in game.c
            if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
                game->player.y + game->player.height + game->velocity.y >= platform.y &&
                game->player.x + game->player.width > platform.x + 8 && 
                game->player.x < platform.x + platform.width - 8) {
                
                game->player.y = platform.y - game->player.height;
                game->velocity.y = 0;
                game->isOnGround = true;
                return i * 100 + j; // Return unique platform ID
            }
        }
    }
    return -1;
}

// Get carried number for player display
int MergeSortGetCarriedNumber(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return 0;
    return data->playerCarrying;
}

bool MergeSortIsCarryingNumber(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return false;
    return data->hasNumber;
}

void MergeSortUpdate(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return;
    
    // Physics collision is now handled in player.c via MergeSortGetPlayerPlatform
    int playerPlatform = GetPlayerPlatform(game);
    
    // Handle input based on current phase
    if (data->currentPhase == PHASE_SPLITTING) {
        // Handle G key for splitting arrays (only focused arrays)
        if (IsKeyPressed(KEY_G)) {
            if (playerPlatform >= 0 && data->subarrayCount == 0 && data->currentFocusIndex == -1) {
                // First split - create initial subarrays from original array (only if focused on original)
                SplitArray(data, -1, game); // -1 means split original array
                UpdateAllPlatforms(data, game);
                printf("Created initial split from original array\n");
                return;
            }
            
            // Check which subarray the player is on
            int playerSubarrayIndex = -1;
            
            // First, determine which subarray the player is on (if any)
            for (int i = 0; i < data->subarrayCount; i++) {
                Subarray* sub = &data->subarrays[i];
                if (!sub->isActive) continue;
                
                for (int j = 0; j < sub->size; j++) {
                    Rectangle platform = sub->platforms[j];
                    if (game->player.x + game->player.width > platform.x + 8 && 
                        game->player.x < platform.x + platform.width - 8 &&
                        abs((int)(game->player.y + game->player.height - platform.y)) < GROUND_TOLERANCE) {
                        playerSubarrayIndex = i;
                        break;
                    }
                }
                if (playerSubarrayIndex >= 0) break;
            }
            
            // Check if player is on the currently focused subarray for splitting
            if (data->currentFocusIndex >= 0 && data->currentFocusIndex < data->subarrayCount) {
                Subarray* focusedSub = &data->subarrays[data->currentFocusIndex];
                
                if (playerSubarrayIndex == data->currentFocusIndex) {
                    // Player is on focused subarray
                    if (!focusedSub->canSplit) {
                        printf("Focused subarray cannot be split (single element or already split)\n");
                        return;
                    }
                    
                    SplitArray(data, data->currentFocusIndex, game);
                    UpdateAllPlatforms(data, game);
                    printf("Split focused subarray %d\n", data->currentFocusIndex);
                    return;
                } else if (playerSubarrayIndex >= 0) {
                    // Player is on a different subarray (not focused)
                    printf("Cannot split non-focused subarray! Current focus is on subarray %d\n", data->currentFocusIndex);
                    return;
                } else {
                    // Player is not on any subarray
                    printf("Must stand on the focused subarray (index %d) to split\n", data->currentFocusIndex);
                    return;
                }
            } else {
                printf("No subarray is currently focused for splitting\n");
            }
        }
    } else if (data->currentPhase == PHASE_MERGING) {
        // Update merge highlights to show which element should be picked
        if (data->highlightingEnabled) {
            UpdateMergeHighlights(data);
        }
        
        // Handle F key for picking/dropping elements
        if (IsKeyPressed(KEY_F)) {
            // Check if player is on original array platform
            if (playerPlatform >= 0) {
                // Player is on original array
                int elementIndex = playerPlatform;
                if (elementIndex < game->arraySize) {
                    if (!data->hasNumber && game->array[elementIndex] != 0) {
                        // Pick up element from original array
                        data->playerCarrying = game->array[elementIndex];
                        data->hasNumber = true;
                        game->array[elementIndex] = 0; // Empty the box
                        printf("Picked up %d from original array\n", data->playerCarrying);
                    } else if (data->hasNumber && game->array[elementIndex] == 0) {
                        // Validate drop position in original array during merge phase
                        bool canDrop = true;
                        
                        if (data->highlightingEnabled && data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
                            Subarray* left = &data->subarrays[data->leftMergeIndex];
                            Subarray* right = &data->subarrays[data->rightMergeIndex];
                            
                            // Check if original array is the target for current merge
                            if (left->parentIndex != -1) {
                                // Target is not original array
                                canDrop = false;
                                printf("Wrong target! Must drop elements in parent subarray %d during this merge.\n", left->parentIndex);
                            } else {
                                // Original array is target - validate sequential placement
                                int expectedPosition = -1;
                                for (int k = 0; k < game->arraySize; k++) {
                                    if (game->array[k] == 0) {
                                        expectedPosition = k;
                                        break;
                                    }
                                }
                                
                                if (elementIndex != expectedPosition) {
                                    canDrop = false;
                                    printf("Wrong position! Must place elements sequentially in first empty box (position %d).\n", expectedPosition);
                                }
                            }
                        }
                        
                        if (canDrop) {
                            // Drop element into empty box in original array
                            game->array[elementIndex] = data->playerCarrying;
                            printf("Dropped %d into original array\n", data->playerCarrying);
                            data->playerCarrying = 0;
                            data->hasNumber = false;
                        } else {
                            // Invalid drop - deduct heart
                            game->hearts--;
                            printf("Invalid placement! Hearts remaining: %d\n", game->hearts);
                            
                            if (game->hearts <= 0) {
                                ChangeState(STATE_GAME_OVER);
                            }
                        }
                    } else if (data->hasNumber && game->array[elementIndex] != 0) {
                        printf("Box is not empty - cannot drop here\n");
                    } else {
                        printf("No number to pick up\n");
                    }
                }
                return;
            }
            
            // Check if player is on any subarray platform
            for (int i = 0; i < data->subarrayCount; i++) {
                Subarray* sub = &data->subarrays[i];
                if (!sub->isActive) continue;
                
                for (int j = 0; j < sub->size; j++) {
                    Rectangle platform = sub->platforms[j];
                    if (game->player.x + game->player.width > platform.x + 8 && 
                        game->player.x < platform.x + platform.width - 8 &&
                        abs((int)(game->player.y + game->player.height - platform.y)) < GROUND_TOLERANCE) {
                        
                        if (!data->hasNumber && sub->elements[j] != 0) {
                            // Check if this is the correct element to pick up during merge phase
                            bool isCorrectChoice = (data->highlightingEnabled && 
                                                  i == data->highlightedSubarray && 
                                                  j == data->highlightedElement);
                            
                            if (data->highlightingEnabled && !isCorrectChoice) {
                                // Player tried to pick up the wrong element - deduct heart and prevent pickup
                                game->hearts--;
                                printf("Wrong choice! You must pick the smaller element first. Hearts remaining: %d\n", game->hearts);
                                
                                if (game->hearts <= 0) {
                                    ChangeState(STATE_GAME_OVER);
                                }
                                return; // Don't allow the pickup
                            }
                            
                            // Pick up element from subarray (correct choice or not in merge highlighting mode)
                            data->playerCarrying = sub->elements[j];
                            data->hasNumber = true;
                            sub->elements[j] = 0; // Empty the box
                            
                            // Advance merge pointers
                            if (i == data->leftMergeIndex && j == data->leftPointer) {
                                data->leftPointer++;
                            } else if (i == data->rightMergeIndex && j == data->rightPointer) {
                                data->rightPointer++;
                            }
                            
                            printf("Picked up %d from subarray\n", data->playerCarrying);
                        } else if (data->hasNumber && sub->elements[j] == 0) {
                            // Validate drop position during merge phase
                            bool canDrop = true;
                            
                            if (data->highlightingEnabled && data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
                                // During merge phase, validate correct placement
                                Subarray* left = &data->subarrays[data->leftMergeIndex];
                                Subarray* right = &data->subarrays[data->rightMergeIndex];
                                
                                // Determine target array (parent of left/right subarrays)
                                bool isTargetArray = false;
                                int expectedPosition = -1;
                                
                                if (left->parentIndex == -1) {
                                    // Target is original array
                                    if (i == -1) { // This would be original array, but we're in subarray loop
                                        // Player is trying to drop in original array - this is handled above
                                        canDrop = false;
                                    } else {
                                        // Player is trying to drop in wrong subarray
                                        canDrop = false;
                                        printf("Wrong target! Must drop elements in the original array during this merge.\n");
                                    }
                                } else {
                                    // Target is parent subarray
                                    if (i == left->parentIndex) {
                                        isTargetArray = true;
                                        // Find the first empty position in target array
                                        for (int k = 0; k < sub->size; k++) {
                                            if (sub->elements[k] == 0) {
                                                expectedPosition = k;
                                                break;
                                            }
                                        }
                                        
                                        // Validate sequential placement - must drop in first empty position
                                        if (j != expectedPosition) {
                                            canDrop = false;
                                            printf("Wrong position! Must place elements sequentially in first empty box (position %d).\n", expectedPosition);
                                        }
                                    } else {
                                        // Player is trying to drop in wrong subarray
                                        canDrop = false;
                                        printf("Wrong target! Must drop elements in the parent array (subarray %d) during this merge.\n", left->parentIndex);
                                    }
                                }
                            }
                            
                            if (canDrop) {
                                // Drop element into empty box in subarray
                                sub->elements[j] = data->playerCarrying;
                                printf("Dropped %d into subarray\n", data->playerCarrying);
                                data->playerCarrying = 0;
                                data->hasNumber = false;
                            } else {
                                // Invalid drop - deduct heart
                                game->hearts--;
                                printf("Invalid placement! Hearts remaining: %d\n", game->hearts);
                                
                                if (game->hearts <= 0) {
                                    ChangeState(STATE_GAME_OVER);
                                }
                            }
                        } else if (data->hasNumber && sub->elements[j] != 0) {
                            printf("Box is not empty - cannot drop here\n");
                        } else {
                            printf("No number to pick up\n");
                        }
                        return;
                    }
                }
            }
            
            printf("Not standing on any platform\n");
        }
        
        // Check if current merge is complete
        if (data->highlightingEnabled && data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
            Subarray* left = &data->subarrays[data->leftMergeIndex];
            Subarray* right = &data->subarrays[data->rightMergeIndex];
            
            // Check if both subarrays are empty (all elements moved to parent)
            bool leftEmpty = true, rightEmpty = true;
            for (int i = 0; i < left->size; i++) {
                if (left->elements[i] != 0) {
                    leftEmpty = false;
                    break;
                }
            }
            for (int i = 0; i < right->size; i++) {
                if (right->elements[i] != 0) {
                    rightEmpty = false;
                    break;
                }
            }
            
            if (leftEmpty && rightEmpty) {
                // Check if parent array is filled
                bool parentFilled = true;
                if (left->parentIndex == -1) {
                    // Parent is original array - check if it's filled
                    for (int k = 0; k < game->arraySize; k++) {
                        if (game->array[k] == 0) {
                            parentFilled = false;
                            break;
                        }
                    }
                } else {
                    // Parent is another subarray - check if it's filled
                    Subarray* parent = &data->subarrays[left->parentIndex];
                    for (int k = 0; k < parent->size; k++) {
                        if (parent->elements[k] == 0) {
                            parentFilled = false;
                            break;
                        }
                    }
                }
                
                if (parentFilled) {
                    // Merge is complete and parent is filled
                    // printf("Merge complete - both subarrays empty and parent filled\n");
                    
                    // Store parent index before cleanup for focus calculation
                    int parentIndex = left->parentIndex;
                    
                    HandleMergeCompletion(data, game);
                    
                    // After merge completion, determine next focus based on merge sort rules
                    if (data->currentPhase == PHASE_SPLITTING) {
                        // The parent array is now filled, check if we need to continue splitting
                        // or if we should merge at the parent level
                        
                        if (parentIndex == -1) {
                            // Merged to original array - check if there are more level 1 subarrays to process
                            bool hasLevel1ToProcess = false;
                            for (int i = 0; i < data->subarrayCount; i++) {
                                Subarray* sub = &data->subarrays[i];
                                if (sub->isActive && sub->size > 1 && sub->level == 1) {
                                    data->currentFocusIndex = i;
                                    hasLevel1ToProcess = true;
                                    printf("Next focus: Level 1 subarray %d\n", i);
                                    break;
                                }
                            }
                            
                            if (!hasLevel1ToProcess) {
                                // All level 1 subarrays processed, merge sort complete
                                data->currentPhase = PHASE_COMPLETE;
                                printf("All level 1 subarrays processed - merge sort complete!\n");
                            }
                        } else {
                            // Merged to a subarray parent - check if parent's sibling needs processing
                            Subarray* parent = &data->subarrays[parentIndex];
                            
                            // If parent is left child, look for right sibling to process
                            if (parent->isLeftChild) {
                                bool foundRightSibling = false;
                                for (int i = 0; i < data->subarrayCount; i++) {
                                    Subarray* sub = &data->subarrays[i];
                                    if (sub->isActive && sub->size > 1 && 
                                        sub->level == parent->level && 
                                        sub->parentIndex == parent->parentIndex &&
                                        !sub->isLeftChild) {
                                        // Found right sibling that needs processing
                                        data->currentFocusIndex = i;
                                        foundRightSibling = true;
                                        printf("Next focus: Right sibling %d at level %d\n", i, sub->level);
                                        break;
                                    }
                                }
                                
                                if (!foundRightSibling) {
                                    // No right sibling, parent level is complete
                                    // Check if parent can be merged with its sibling
                                    FindMergeablePairs(data);
                                    if (data->highlightingEnabled) {
                                        data->currentPhase = PHASE_MERGING;
                                        printf("Parent level complete, starting merge at parent level\n");
                                    } else {
                                        // Continue up the tree
                                        int nextFocus = FindNextFocusAfterMerge(data, parent->parentIndex);
                                        if (nextFocus >= 0) {
                                            data->currentFocusIndex = nextFocus;
                                            printf("Continuing up tree, next focus: %d\n", nextFocus);
                                        } else {
                                            data->currentPhase = PHASE_COMPLETE;
                                            printf("No more processing needed - complete!\n");
                                        }
                                    }
                                }
                            } else {
                                // Parent is right child, its subtree is complete
                                // Check if parent can be merged with left sibling
                                FindMergeablePairs(data);
                                if (data->highlightingEnabled) {
                                    data->currentPhase = PHASE_MERGING;
                                    printf("Right subtree complete, merging at parent level\n");
                                } else {
                                    // Continue up the tree
                                    int nextFocus = FindNextFocusAfterMerge(data, parent->parentIndex);
                                    if (nextFocus >= 0) {
                                        data->currentFocusIndex = nextFocus;
                                        printf("Continuing up tree, next focus: %d\n", nextFocus);
                                    } else {
                                        data->currentPhase = PHASE_COMPLETE;
                                        printf("No more processing needed - complete!\n");
                                    }
                                }
                            }
                        }
                        
                        PrintMergeSortState(data);
                    }
                } else {
                    // printf("Subarrays empty but parent not filled yet\n");
                }
            }
        }
    }
    

    
    // Update focus states for all subarrays
    UpdateSubarrayFocusStates(data);
    
    // Check if we should transition to merge phase
    if (data->currentPhase == PHASE_SPLITTING && data->recursionComplete) {
        data->currentPhase = PHASE_MERGING;
        data->highlightingEnabled = true; // Enable merge highlighting
        FindMergeablePairs(data); // Find first mergeable pair
        printf("Transitioning to merge phase - recursion complete\n");
    }
    
    // Integrate with existing camera system
    UpdateMergeSortCamera(game, data);
    
    // Check for completion
    if (MergeSortIsComplete(game)) {
        data->currentPhase = PHASE_COMPLETE;
        game->gameComplete = true;
    }
    
    // Handle game over condition
    if (game->hearts <= 0) {
        ChangeState(STATE_GAME_OVER);
    }
    
    // Handle level completion
    if (game->gameComplete && data->currentPhase == PHASE_COMPLETE && IsReadyForCompletion(game)) {
        StartCompletionAnimation();
    }
}

void MergeSortGetStats(GameData* game, AlgorithmStats* stats) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return;
    
    // Primary stat: Phase information
    const char* phaseText;
    switch (data->currentPhase) {
        case PHASE_SPLITTING:
            phaseText = "PHASE: SPLITTING (Divide)";
            break;
        case PHASE_MERGING:
            phaseText = "PHASE: MERGING (Conquer)";
            break;
        case PHASE_COMPLETE:
            phaseText = "MERGE SORT COMPLETE!";
            break;
        default:
            phaseText = "PHASE: Unknown";
            break;
    }
    strcpy(stats->primaryStat, phaseText);
    
    // Secondary stat: Statistics
    sprintf(stats->secondaryStat, "Splits: %d | Merges: %d | Max Level: %d | Recursion: %s", 
            data->splitCount, data->mergeCount, data->maxLevel, 
            data->recursionComplete ? "Complete" : "In Progress");
    
    // Instructions based on current phase and state
    stats->hasInstruction = true;
    if (data->currentPhase == PHASE_SPLITTING) {
        if (data->currentFocusIndex == -1 && data->subarrayCount == 0) {
            strcpy(stats->instructionText, "Stand on original array and press G to begin depth-first splitting");
            stats->instructionColor = YELLOW;
        } else if (data->currentFocusIndex >= 0 && data->currentFocusIndex < data->subarrayCount) {
            Subarray* focused = &data->subarrays[data->currentFocusIndex];
            if (focused->canSplit) {
                strcpy(stats->instructionText, "Stand on FOCUSED subarray (highlighted) and press G to split");
                stats->instructionColor = YELLOW;
            } else {
                strcpy(stats->instructionText, "Focused subarray is single element - focus will move automatically");
                stats->instructionColor = UI_TEXT_PRIMARY;
            }
        } else {
            strcpy(stats->instructionText, "Recursion complete - transitioning to merge phase");
            stats->instructionColor = YELLOW;
        }
    } else if (data->currentPhase == PHASE_MERGING) {
        if (data->hasNumber) {
            sprintf(stats->instructionText, "Carrying: %d - Press F to drop in correct position", data->playerCarrying);
            stats->instructionColor = YELLOW;
        } else {
            strcpy(stats->instructionText, "Press F to pick up numbers for merging");
            stats->instructionColor = UI_TEXT_PRIMARY;
        }
    } else {
        stats->hasInstruction = false;
    }
    
    strcpy(stats->goalText, "Goal: Sort using divide-and-conquer merge sort algorithm");
}

void MergeSortRender(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return;
    
    // The original array is rendered by game.c - we just render additional subarrays
    
    // Find which specific subarray platform the player is on (if any)
    int playerSubarrayIndex = -1;
    int playerElementIndex = -1;
    
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (!sub->isActive) continue;
        
        for (int j = 0; j < sub->size; j++) {
            Rectangle platform = sub->platforms[j];
            if (game->player.x + game->player.width > platform.x + 8 && 
                game->player.x < platform.x + platform.width - 8 &&
                abs((int)(game->player.y + game->player.height - platform.y)) < GROUND_TOLERANCE) {
                playerSubarrayIndex = i;
                playerElementIndex = j;
                break; // Found the platform, stop searching
            }
        }
        if (playerSubarrayIndex >= 0) break; // Found the platform, stop searching
    }
    
    // Render all active subarrays (positioned above original array)
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (!sub->isActive) continue;
        
        // Draw subarray platforms and elements with same style as original
        for (int j = 0; j < sub->size; j++) {
            Rectangle platform = sub->platforms[j];
            
            // Check if this is the specific platform the player is on
            bool playerOnPlatform = (playerSubarrayIndex == i && playerElementIndex == j);
            
            // Check different highlight states
            bool isCorrectChoice = (data->highlightingEnabled && 
                                  i == data->highlightedSubarray && 
                                  j == data->highlightedElement);
            
            bool isLeftCompare = (data->highlightingEnabled && 
                                i == data->leftMergeIndex && 
                                j == data->leftCompareElement);
            
            bool isRightCompare = (data->highlightingEnabled && 
                                 i == data->rightMergeIndex && 
                                 j == data->rightCompareElement);
            
            // Check if this subarray is currently focused
            bool isFocused = IsFocused(data, i);
            
            // Visual feedback with different colors for comparison
            Color platformColor;
            Color borderColor;
            
            if (isCorrectChoice) {
                // Correct choice - bright yellow
                platformColor = YELLOW;
                borderColor = ORANGE;
            } else if (isLeftCompare || isRightCompare) {
                // Elements being compared - light blue outline
                platformColor = UI_BUTTON_NORMAL;
                borderColor = SKYBLUE;
            } else {
                // Normal platform color
                platformColor = UI_BUTTON_NORMAL;
                if (isFocused) {
                    // Simple blue outline for focused subarray
                    borderColor = BLUE;
                } else if (playerOnPlatform) {
                    // Light green outline when player is standing on platform
                    borderColor = LIME;
                } else {
                    borderColor = LIGHTGRAY;
                }
            }
            
            DrawRectangleRec(platform, platformColor);
            DrawRectangleLinesEx(platform, 2, borderColor);
            
            // Draw element number with same style as original (0 means empty box)
            if (sub->elements[j] != 0) {
                char numText[8];
                sprintf(numText, "%d", sub->elements[j]);
                Color textColor = isCorrectChoice ? BLACK : UI_TEXT_PRIMARY;
                DrawCenteredText(numText, 
                               platform.x + platform.width/2, 
                               platform.y + platform.height/2, 
                               FONT_SIZE_BUTTON * 1.5, textColor);
            } else {
                // Draw empty box indicator
                Color emptyColor = isCorrectChoice ? DARKGRAY : GRAY;
                DrawText(" ", 
                        platform.x + platform.width/2 - 8, 
                        platform.y + platform.height/2 - 8, 
                        FONT_SIZE_BUTTON, emptyColor);
            }
        }
        
        // Draw level indicator
        char levelText[16];
        sprintf(levelText, "L%d", sub->level);
        DrawText(levelText, sub->position.x - 30, sub->position.y + 20, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
        

    }
    
    // Draw phase information and statistics
    const char* phaseText = "";
    switch (data->currentPhase) {
        case PHASE_SPLITTING:
            if (data->subarrayCount == 0) {
                phaseText = "SPLITTING PHASE - Stand on original array and press G to start splitting";
            } else {
                phaseText = "SPLITTING PHASE - Stand on subarrays and press G to split further";
            }
            break;
        case PHASE_MERGING:
            if (data->highlightingEnabled) {
                phaseText = "MERGING PHASE - Compare outlined elements, pick YELLOW (smaller) one first";
            } else {
                phaseText = "MERGING PHASE - Use F to pick up and drop numbers to merge subarrays";
            }
            break;
        case PHASE_COMPLETE:
            phaseText = "MERGE SORT COMPLETE!";
            break;
    }
    
    // Text rendering moved to MergeSortGetStats function
}

void MergeSortCleanup(GameData* game) {
    if (!game || !game->algorithmData) return;
    
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    
    // Clean up all subarrays with proper null checks
    if (data->subarrays) {
        for (int i = 0; i < data->subarrayCount; i++) {
            if (data->subarrays[i].elements) {
                free(data->subarrays[i].elements);
                data->subarrays[i].elements = NULL;
            }
            if (data->subarrays[i].platforms) {
                free(data->subarrays[i].platforms);
                data->subarrays[i].platforms = NULL;
            }
        }
        free(data->subarrays);
        data->subarrays = NULL;
    }
    
    // Clean up original active tracking
    if (data->originalActive) {
        free(data->originalActive);
        data->originalActive = NULL;
    }
    
    // Clean up focus system
    if (data->focusStack) {
        free(data->focusStack);
        data->focusStack = NULL;
    }
    
    // Reset camera to default behavior for other algorithms
    game->camera.target = (Vector2){ 
        game->player.x + game->player.width/2, 
        game->player.y + game->player.height/2 
    };
    
    // Free the main data structure
    free(data);
    game->algorithmData = NULL;
    
    printf("Merge Sort cleaned up with proper memory management\n");
}

bool MergeSortIsComplete(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return false;
    
    // Merge sort is only complete when:
    // 1. We've finished all merging operations
    // 2. The original array has been filled back with sorted values (no zeros)
    // 3. The array is actually sorted
    
    // Check if original array has any empty slots (zeros)
    for (int i = 0; i < game->arraySize; i++) {
        if (game->array[i] == 0) {
            return false; // Still has empty slots, not complete
        }
    }
    
    // Check if array is properly sorted
    return IsArraySorted(game->array, game->arraySize, true);
}

void MergeSortResetLevel(GameData* game, int level) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) {
        printf("Error: MergeSortData not initialized\n");
        return;
    }
    
    // Clean up existing subarrays with proper error checking
    for (int i = 0; i < data->subarrayCount; i++) {
        if (data->subarrays[i].elements) {
            free(data->subarrays[i].elements);
            data->subarrays[i].elements = NULL;
        }
        if (data->subarrays[i].platforms) {
            free(data->subarrays[i].platforms);
            data->subarrays[i].platforms = NULL;
        }
    }
    
    // Reset state to initial conditions
    data->subarrayCount = 0;
    data->currentPhase = PHASE_SPLITTING;
    data->maxLevel = 0;
    data->recursionComplete = false;
    data->splitCount = 0;
    data->mergeCount = 0;
    data->cameraFollowPlayer = true;
    
    // Reset merge state
    data->leftMergeIndex = -1;
    data->rightMergeIndex = -1;
    data->targetMergeIndex = -1;
    data->leftPointer = 0;
    data->rightPointer = 0;
    
    // Reset selection state
    data->highlightingEnabled = false;
    data->highlightedElement = -1;
    data->highlightedSubarray = -1;
    
    // Reset player carrying system
    data->playerCarrying = 0;
    data->hasNumber = false;
    
    // Reset focus system
    data->currentFocusIndex = -1; // Start focused on original array
    data->focusStackSize = 0;
    
    // Set up base array based on level - use the original game array
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
    
    // Initialize original active tracking - all elements start as active
    for (int i = 0; i < game->arraySize; i++) {
        data->originalActive[i] = true;
    }
    
    // Reset camera to follow player at base level
    Vector2 playerCenter = { 
        game->player.x + game->player.width/2, 
        game->player.y + game->player.height/2 
    };
    data->cameraTarget = playerCenter;
    game->camera.target = playerCenter;
    
    printf("Merge Sort level %d reset with %d elements - working with original array\n", level, game->arraySize);
}
// Helper function implementations

static void AddSubarray(MergeSortData* data, Subarray subarray) {
    // Resize array if needed
    if (data->subarrayCount >= data->subarrayCapacity) {
        data->subarrayCapacity *= 2;
        data->subarrays = (Subarray*)realloc(data->subarrays, 
                                           data->subarrayCapacity * sizeof(Subarray));
    }
    
    data->subarrays[data->subarrayCount] = subarray;
    data->subarrayCount++;
}

static Vector2 CalculateSubarrayPosition(MergeSortData* data, GameData* game, int level, int parentIndex, bool isLeftChild, int elementsInSubarray) {
    float levelHeight = 80.0f * 1.5; // Vertical spacing between levels
    float baseY = ARRAY_Y_POSITION - (level * levelHeight);
    
    float startX;
    
    if (level == 1) {
        // Level 1: Position relative to original array
        float originalArrayStartX = (SCREEN_WIDTH - game->arraySize * BOX_SIZE) / 2;
        float originalArrayEndX = originalArrayStartX + (game->arraySize * BOX_SIZE);
        
        if (isLeftChild) {
            // Left subarray ends at left edge of original array
            startX = originalArrayStartX - (elementsInSubarray * BOX_SIZE);
        } else {
            // Right subarray starts at right edge of original array
            startX = originalArrayEndX;
        }
    } else {
        // Higher levels: Position relative to parent subarray
        if (parentIndex >= 0 && parentIndex < data->subarrayCount) {
            Subarray* parent = &data->subarrays[parentIndex];
            float parentStartX = parent->position.x;
            float parentEndX = parentStartX + (parent->size * BOX_SIZE);
            
            if (isLeftChild) {
                // Left child ends at left edge of parent
                startX = parentStartX - (elementsInSubarray * BOX_SIZE);
            } else {
                // Right child starts at right edge of parent
                startX = parentEndX;
            }
        } else {
            // Fallback to center positioning if parent not found
            startX = (SCREEN_WIDTH - elementsInSubarray * BOX_SIZE) / 2;
        }
    }
    
    return (Vector2){ startX, baseY };
}

static void UpdateAllPlatforms(MergeSortData* data, GameData* game) {
    // Update positions for all subarrays using parent-based positioning
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (!sub->isActive) continue;
        
        // Use the parent-based positioning function
        Vector2 pos = CalculateSubarrayPosition(data, game, sub->level, sub->parentIndex, sub->isLeftChild, sub->size);
        sub->position = pos;
        
        // Create platform rectangles for this subarray
        for (int j = 0; j < sub->size; j++) {
            sub->platforms[j] = (Rectangle){
                pos.x + j * BOX_SIZE,
                pos.y,
                BOX_SIZE,
                BOX_SIZE
            };
        }
    }
}

static void SplitArray(MergeSortData* data, int subarrayIndex, GameData* game) {
    if (subarrayIndex == -1) {
        // First split - create subarrays from original array
        int mid = game->arraySize / 2;
        int newLevel = 1;
        
        // Create left subarray
        Subarray left = {
            .elements = (int*)malloc(mid * sizeof(int)),
            .size = mid,
            .level = newLevel,
            .index = 0,     // Left child
            .parentIndex = -1, // Parent is original array
            .isActive = true,
            .canSplit = (mid > 1),
            .isExhausted = false,
            .isLeftChild = true,
            .platforms = (Rectangle*)malloc(mid * sizeof(Rectangle))
        };
        memcpy(left.elements, game->array, mid * sizeof(int));
        
        // Create right subarray
        Subarray right = {
            .elements = (int*)malloc((game->arraySize - mid) * sizeof(int)),
            .size = game->arraySize - mid,
            .level = newLevel,
            .index = 1, // Right child
            .parentIndex = -1, // Parent is original array
            .isActive = true,
            .canSplit = ((game->arraySize - mid) > 1),
            .isExhausted = false,
            .isLeftChild = false,
            .platforms = (Rectangle*)malloc((game->arraySize - mid) * sizeof(Rectangle))
        };
        memcpy(right.elements, game->array + mid, (game->arraySize - mid) * sizeof(int));
        
        // Clear original array elements to show they'll be filled from bottom up
        for (int i = 0; i < game->arraySize; i++) {
            game->array[i] = 0; // 0 means empty box
        }
        
        // Add new subarrays
        AddSubarray(data, left);
        AddSubarray(data, right);
        
        data->maxLevel = newLevel;
        data->splitCount++;
        
        // Move focus to left child first (depth-first left traversal)
        MoveFocusAfterSplit(data, 0, 1); // Left child index 0, right child index 1
        
        printf("Initial split of original array into 2 subarrays, focus moved to left child\n");
        return;
    }
    
    // Split existing subarray
    Subarray* parent = &data->subarrays[subarrayIndex];
    
    if (parent->size <= 1) return; // Cannot split single elements
    
    int mid = parent->size / 2;
    int newLevel = parent->level + 1;
    
    // Create left subarray
    Subarray left = {
        .elements = (int*)malloc(mid * sizeof(int)),
        .size = mid,
        .level = newLevel,
        .index = parent->index * 2,     // Left child
        .parentIndex = subarrayIndex,   // Parent subarray index
        .isActive = true,
        .canSplit = (mid > 1),
        .isExhausted = false,
        .isLeftChild = true,
        .platforms = (Rectangle*)malloc(mid * sizeof(Rectangle))
    };
    memcpy(left.elements, parent->elements, mid * sizeof(int));
    
    // Create right subarray
    Subarray right = {
        .elements = (int*)malloc((parent->size - mid) * sizeof(int)),
        .size = parent->size - mid,
        .level = newLevel,
        .index = parent->index * 2 + 1, // Right child
        .parentIndex = subarrayIndex,   // Parent subarray index
        .isActive = true,
        .canSplit = ((parent->size - mid) > 1),
        .isExhausted = false,
        .isLeftChild = false,
        .platforms = (Rectangle*)malloc((parent->size - mid) * sizeof(Rectangle))
    };
    memcpy(right.elements, parent->elements + mid, (parent->size - mid) * sizeof(int));
    
    // Clear parent elements to show they'll be filled from bottom up after recursion
    for (int i = 0; i < parent->size; i++) {
        parent->elements[i] = 0; // 0 means empty box
    }
    parent->canSplit = false;
    // Keep parent visible to show the empty slots
    
    // Store indices of new children before adding them
    int leftChildIndex = data->subarrayCount;
    int rightChildIndex = data->subarrayCount + 1;
    
    // Add new subarrays
    AddSubarray(data, left);
    AddSubarray(data, right);
    
    // Update max level
    if (newLevel > data->maxLevel) {
        data->maxLevel = newLevel;
    }
    
    data->splitCount++;
    
    // Move focus to left child first (depth-first left traversal)
    MoveFocusAfterSplit(data, leftChildIndex, rightChildIndex);
    
    printf("Split subarray at level %d, now have %d active subarrays, focus moved to left child\n", 
           newLevel, data->subarrayCount);
    
    // After splitting, check if the left child is now a single element
    // If so, we need to move focus to the right child
    if (leftChildIndex < data->subarrayCount && data->subarrays[leftChildIndex].size == 1) {
        // Left child is single element, move focus to right child if it can be split
        if (rightChildIndex < data->subarrayCount && data->subarrays[rightChildIndex].size > 1) {
            data->currentFocusIndex = rightChildIndex;
            printf("Left child is single element, focus moved to right child (index %d)\n", rightChildIndex);
        } else {
            // Both children are single elements, move focus back up
            UpdateFocus(data);
        }
    }
    
    // Update focus system to check if recursion is complete
    UpdateFocus(data);
}



static void UpdateMergeSortCamera(GameData* game, MergeSortData* data) {
    if (!data->cameraFollowPlayer) return;
    
    Vector2 playerCenter = { 
        game->player.x + game->player.width/2, 
        game->player.y + game->player.height/2 
    };
    
    Vector2 targetPos = playerCenter;
    
    // Adjust target based on phase for better visibility
    if (data->currentPhase == PHASE_SPLITTING) {
        // Look slightly upward during splitting to see new subarrays
        targetPos.y -= 100;
    } else if (data->currentPhase == PHASE_MERGING) {
        // Look slightly downward during merging to see merge targets
        targetPos.y += 50;
    }
    
    // Store target for integration with existing camera system
    data->cameraTarget = targetPos;
    
    // Let the existing camera system handle the smooth movement
    // We just provide guidance by setting the camera target
    // The existing UpdateGameCamera function in game.c will handle smooth following
    game->camera.target = targetPos;
}

static void StartMergePhase(MergeSortData* data) {
    // Reset merge state
    data->leftMergeIndex = -1;
    data->rightMergeIndex = -1;
    data->leftPointer = 0;
    data->rightPointer = 0;
    
    // Find the first mergeable pair
    FindMergeablePairs(data);
}

static void UpdateMergeHighlights(MergeSortData* data) {
    if (!data->highlightingEnabled) return;
    
    // Reset highlights
    data->highlightedElement = -1;
    data->highlightedSubarray = -1;
    data->leftCompareElement = -1;
    data->rightCompareElement = -1;
    
    // Show both elements being compared
    if (data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
        Subarray* left = &data->subarrays[data->leftMergeIndex];
        Subarray* right = &data->subarrays[data->rightMergeIndex];
        
        if (data->leftPointer < left->size && data->rightPointer < right->size) {
            // Both have elements - show both for comparison
            data->leftCompareElement = data->leftPointer;
            data->rightCompareElement = data->rightPointer;
            
            // Highlight the smaller one (correct choice) in yellow
            int leftVal = left->elements[data->leftPointer];
            int rightVal = right->elements[data->rightPointer];
            
            if (leftVal <= rightVal) {
                data->highlightedSubarray = data->leftMergeIndex;
                data->highlightedElement = data->leftPointer;
            } else {
                data->highlightedSubarray = data->rightMergeIndex;
                data->highlightedElement = data->rightPointer;
            }
        } else if (data->leftPointer < left->size) {
            // Only left has elements remaining
            data->highlightedSubarray = data->leftMergeIndex;
            data->highlightedElement = data->leftPointer;
        } else if (data->rightPointer < right->size) {
            // Only right has elements remaining
            data->highlightedSubarray = data->rightMergeIndex;
            data->highlightedElement = data->rightPointer;
        }
    }
}

static bool ProcessMergeChoice(GameData* game, MergeSortData* data, int subarrayIndex, int elementIndex) {
    // Check if this is the correct choice
    bool isCorrect = (subarrayIndex == data->highlightedSubarray && 
                     elementIndex == data->highlightedElement);
    
    if (isCorrect) {
        Subarray* sub = &data->subarrays[subarrayIndex];
        int element = sub->elements[elementIndex];
        
        // Move element to parent array
        Subarray* left = &data->subarrays[data->leftMergeIndex];
        Subarray* right = &data->subarrays[data->rightMergeIndex];
        
        // Find target position in parent array
        int targetPosition = data->leftPointer + data->rightPointer;
        
        if (left->parentIndex == -1) {
            // Parent is original array
            game->array[targetPosition] = element;
        } else {
            // Parent is another subarray
            Subarray* parent = &data->subarrays[left->parentIndex];
            parent->elements[targetPosition] = element;
        }
        
        // Mark element as used (set to 0)
        sub->elements[elementIndex] = 0;
        
        // Advance the appropriate pointer
        if (subarrayIndex == data->leftMergeIndex) {
            data->leftPointer++;
        } else if (subarrayIndex == data->rightMergeIndex) {
            data->rightPointer++;
        }
        
        // Increment merge count
        data->mergeCount++;
        
        printf("Selected element %d correctly, moved to parent position %d\n", element, targetPosition);
        
        // Check if this merge pair is complete
        if (data->leftPointer >= left->size && data->rightPointer >= right->size) {
            // Mark subarrays as exhausted
            left->isExhausted = true;
            right->isExhausted = true;
            
            printf("Merge pair complete - subarrays exhausted\n");
            
            // Find next mergeable pair
            FindMergeablePairs(data);
            
            // If no more pairs, check if merge phase is complete
            if (!data->highlightingEnabled) {
                // Remove exhausted subarrays
                for (int i = data->subarrayCount - 1; i >= 0; i--) {
                    if (data->subarrays[i].isExhausted) {
                        // Free memory
                        free(data->subarrays[i].elements);
                        free(data->subarrays[i].platforms);
                        
                        // Remove from array by shifting
                        for (int j = i; j < data->subarrayCount - 1; j++) {
                            data->subarrays[j] = data->subarrays[j + 1];
                        }
                        data->subarrayCount--;
                    }
                }
                
                printf("Cleaned up exhausted subarrays, %d remaining\n", data->subarrayCount);
                
                // Update platform positions after cleanup
                UpdateAllPlatforms(data, game);
                
                // Check if we need to continue merging or if we're done
                if (data->subarrayCount == 0) {
                    // All subarrays merged back to original array
                    data->currentPhase = PHASE_COMPLETE;
                    printf("Merge sort complete!\n");
                } else {
                    // Continue merging at lower levels
                    FindMergeablePairs(data);
                }
            }
        }
        
        return true;
    } else {
        // Incorrect choice - deduct heart
        game->hearts--;
        printf("Wrong choice! Hearts remaining: %d\n", game->hearts);
        
        if (game->hearts <= 0) {
            ChangeState(STATE_GAME_OVER);
        }
        
        return false;
    }
}

static bool CanMerge(MergeSortData* data, int leftIndex, int rightIndex) {
    if (leftIndex < 0 || rightIndex < 0 || 
        leftIndex >= data->subarrayCount || rightIndex >= data->subarrayCount) {
        return false;
    }
    
    Subarray* left = &data->subarrays[leftIndex];
    Subarray* right = &data->subarrays[rightIndex];
    
    // Can merge if:
    // 1. Both are active and not exhausted
    // 2. They have the same parent (either same parentIndex or both have parentIndex -1)
    // 3. Left is left child and right is right child
    return left->isActive && right->isActive && 
           !left->isExhausted && !right->isExhausted &&
           left->parentIndex == right->parentIndex &&
           left->isLeftChild && !right->isLeftChild;
}

static void FindMergeablePairs(MergeSortData* data) {
    // Find pairs of subarrays that can be merged
    // Start from the highest level and work down
    for (int level = data->maxLevel; level >= 1; level--) {
        for (int i = 0; i < data->subarrayCount - 1; i++) {
            for (int j = i + 1; j < data->subarrayCount; j++) {
                if (data->subarrays[i].level == level && 
                    data->subarrays[j].level == level &&
                    CanMerge(data, i, j)) {
                    
                    // Found a mergeable pair
                    data->leftMergeIndex = i;
                    data->rightMergeIndex = j;
                    data->leftPointer = 0;
                    data->rightPointer = 0;
                    data->highlightingEnabled = true;
                    
                    printf("Found mergeable pair: subarrays %d and %d at level %d\n", i, j, level);
                    return;
                }
            }
        }
    }
    
    // No mergeable pairs found
    data->highlightingEnabled = false;
    printf("No mergeable pairs found\n");
}

static void InitializeFocusSystem(MergeSortData* data) {
    data->currentFocusIndex = -1; // Start focused on original array
    data->focusStackCapacity = 16; // Initial capacity
    data->focusStack = (int*)malloc(data->focusStackCapacity * sizeof(int));
    data->focusStackSize = 0;
    
    if (!data->focusStack) {
        printf("Error: Failed to allocate focus stack\n");
        data->focusStackCapacity = 0;
    }
}

static void PushFocusStack(MergeSortData* data, int focusIndex) {
    if (data->focusStackSize >= data->focusStackCapacity) {
        // Resize stack if needed
        data->focusStackCapacity *= 2;
        data->focusStack = (int*)realloc(data->focusStack, data->focusStackCapacity * sizeof(int));
    }
    
    data->focusStack[data->focusStackSize] = focusIndex;
    data->focusStackSize++;
    printf("Pushed focus %d onto stack (size: %d)\n", focusIndex, data->focusStackSize);
}

static int PopFocusStack(MergeSortData* data) {
    if (data->focusStackSize <= 0) {
        return -1; // Stack empty
    }
    
    data->focusStackSize--;
    int focusIndex = data->focusStack[data->focusStackSize];
    printf("Popped focus %d from stack (size: %d)\n", focusIndex, data->focusStackSize);
    return focusIndex;
}

static void MoveFocusAfterSplit(MergeSortData* data, int leftChildIndex, int rightChildIndex) {
    // Push current focus onto stack for later return
    if (data->currentFocusIndex != -1) {
        PushFocusStack(data, data->currentFocusIndex);
    }
    

    
    // Always focus on left child first (depth-first left traversal)
    data->currentFocusIndex = leftChildIndex;
    printf("Focus moved to left child (index %d), right child is %d\n", leftChildIndex, rightChildIndex);
    
    // Update focus states for all subarrays
    UpdateSubarrayFocusStates(data);
}

static bool IsFocused(MergeSortData* data, int subarrayIndex) {
    return data->currentFocusIndex == subarrayIndex;
}

static int FindSibling(MergeSortData* data, int subarrayIndex) {
    if (subarrayIndex < 0 || subarrayIndex >= data->subarrayCount) {
        return -1;
    }
    
    Subarray* target = &data->subarrays[subarrayIndex];
    
    // Find sibling with same parent
    for (int i = 0; i < data->subarrayCount; i++) {
        if (i == subarrayIndex) continue;
        
        Subarray* candidate = &data->subarrays[i];
        if (candidate->parentIndex == target->parentIndex && 
            candidate->level == target->level &&
            candidate->isLeftChild != target->isLeftChild) {
            return i;
        }
    }
    
    return -1; // No sibling found
}

static void UpdateFocus(MergeSortData* data) {
    if (data->currentFocusIndex == -1) {
        // Focused on original array, nothing to update during splitting phase
        return;
    }
    
    if (data->currentFocusIndex >= data->subarrayCount) {
        // Invalid focus index, reset to original array
        data->currentFocusIndex = -1;
        return;
    }
    
    Subarray* focused = &data->subarrays[data->currentFocusIndex];
    
    // If current focused subarray is single element, move focus according to depth-first rules
    if (focused->size == 1) {
        printf("Current focus (index %d) is single element, determining next focus...\n", data->currentFocusIndex);
        
        // Find sibling of current focused subarray
        int siblingIndex = FindSibling(data, data->currentFocusIndex);
        
        if (focused->isLeftChild) {
            // We're on left child, move to right sibling if it exists and can be split
            if (siblingIndex >= 0) {
                Subarray* sibling = &data->subarrays[siblingIndex];
                if (sibling->size > 1) {
                    // Right sibling can still be split, focus on it
                    data->currentFocusIndex = siblingIndex;
                    printf("Left child is single element, focus moved to right child (index %d)\n", siblingIndex);
                    UpdateSubarrayFocusStates(data);
                    return;
                } else {
                    // Right sibling is also single element, both children complete
                    // Start merge phase for these two single elements
                    printf("Both children are single elements, starting merge phase\n");
                    
                    // Set up merge state for these two single elements
                    data->leftMergeIndex = data->currentFocusIndex;  // Left child (current focus)
                    data->rightMergeIndex = siblingIndex;            // Right sibling
                    data->leftPointer = 0;
                    data->rightPointer = 0;
                    data->highlightingEnabled = true;
                    
                    // Switch to merge phase for this pair
                    data->currentPhase = PHASE_MERGING;
                    data->currentFocusIndex = -1; // No focus during merging
                    
                    printf("Started merging subarrays %d and %d\n", data->leftMergeIndex, data->rightMergeIndex);
                    return;
                }
            } else {
                // No right sibling found, this shouldn't happen in proper merge sort
                printf("Warning: Left child has no right sibling\n");
            }
        } else {
            // We're on right child that's complete, check if we can merge with left sibling
            int leftSiblingIndex = FindSibling(data, data->currentFocusIndex);
            if (leftSiblingIndex >= 0 && data->subarrays[leftSiblingIndex].size == 1) {
                // Both siblings are single elements, start merge phase
                printf("Both children are single elements, starting merge phase\n");
                
                data->leftMergeIndex = leftSiblingIndex;           // Left sibling
                data->rightMergeIndex = data->currentFocusIndex;   // Right child (current focus)
                data->leftPointer = 0;
                data->rightPointer = 0;
                data->highlightingEnabled = true;
                
                // Switch to merge phase for this pair
                data->currentPhase = PHASE_MERGING;
                data->currentFocusIndex = -1; // No focus during merging
                
                printf("Started merging subarrays %d and %d\n", data->leftMergeIndex, data->rightMergeIndex);
                return;
            } else {
                // Right child complete but left sibling not single element yet
                // This means left sibling is still being processed, wait
                printf("Right child complete, waiting for left sibling to complete\n");
                data->currentFocusIndex = -1; // No focus while waiting
                return;
            }
        }
    }
    
    // Check if all subarrays are single elements (recursion complete)
    bool allSingleElements = true;
    for (int i = 0; i < data->subarrayCount; i++) {
        if (data->subarrays[i].isActive && data->subarrays[i].size > 1) {
            allSingleElements = false;
            break;
        }
    }
    
    if (allSingleElements && !data->recursionComplete) {
        data->recursionComplete = true;
        data->currentFocusIndex = -1; // No focus needed during merge phase
        printf("All subarrays are single elements - recursion complete, ready for merging\n");
    }
    
    // Update focus states for all subarrays
    UpdateSubarrayFocusStates(data);
}

static void UpdateSubarrayFocusStates(MergeSortData* data) {
    // Update canSplit property based on focus - only focused subarray can be split
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (sub->isActive && sub->size > 1) {
            // Only allow splitting if this subarray is currently focused
            sub->canSplit = (i == data->currentFocusIndex);
        } else {
            // Single elements cannot be split regardless of focus
            sub->canSplit = false;
        }
    }
}

static void HandleMergeCompletion(MergeSortData* data, GameData* game) {
    // Called when a merge pair is completed
    // Remove the merged subarrays and continue with next pair or return to splitting
    
    if (data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
        // Mark merged subarrays as exhausted
        data->subarrays[data->leftMergeIndex].isExhausted = true;
        data->subarrays[data->rightMergeIndex].isExhausted = true;
        
        printf("Merge completed for subarrays %d and %d\n", data->leftMergeIndex, data->rightMergeIndex);
        
        // Clean up exhausted subarrays
        for (int i = data->subarrayCount - 1; i >= 0; i--) {
            if (data->subarrays[i].isExhausted) {
                // Free memory
                free(data->subarrays[i].elements);
                free(data->subarrays[i].platforms);
                
                // Remove from array by shifting
                for (int j = i; j < data->subarrayCount - 1; j++) {
                    data->subarrays[j] = data->subarrays[j + 1];
                }
                data->subarrayCount--;
            }
        }
        
        printf("Cleaned up exhausted subarrays, %d remaining\n", data->subarrayCount);
        
        // Reset merge state
        data->leftMergeIndex = -1;
        data->rightMergeIndex = -1;
        data->highlightingEnabled = false;
        
        // Check if there are more subarrays to process
        bool hasMultiElementSubarrays = false;
        for (int i = 0; i < data->subarrayCount; i++) {
            if (data->subarrays[i].isActive && data->subarrays[i].size > 1) {
                hasMultiElementSubarrays = true;
                break;
            }
        }
        
        if (hasMultiElementSubarrays) {
            // Return to splitting phase - focus will be set by caller
            data->currentPhase = PHASE_SPLITTING;
        } else {
            // All subarrays are single elements or we're done
            if (data->subarrayCount == 0) {
                // All merged back to original array
                data->currentPhase = PHASE_COMPLETE;
                printf("Merge sort complete!\n");
            } else {
                // Find more mergeable pairs
                FindMergeablePairs(data);
                if (!data->highlightingEnabled) {
                    // No more pairs, complete
                    data->currentPhase = PHASE_COMPLETE;
                    printf("No more mergeable pairs - merge sort complete!\n");
                }
            }
        }
        
        // Update platform positions after cleanup
        UpdateAllPlatforms(data, game);
    }
}

static int FindNextFocusAfterMerge(MergeSortData* data, int mergedParentIndex) {
    // Find the next subarray to focus on after completing a merge
    // Following proper depth-first merge sort recursion order
    
    // After a merge completes, we need to check the focus stack to see where we came from
    // The proper order is: complete left subtree, then right subtree, then merge parent
    
    // Pop from focus stack to get the parent that was being processed
    if (data->focusStackSize > 0) {
        int parentFocus = PopFocusStack(data);
        
        // Check if this parent has a right sibling that needs processing
        if (parentFocus >= 0 && parentFocus < data->subarrayCount) {
            Subarray* parent = &data->subarrays[parentFocus];
            
            // If parent is a left child, look for its right sibling
            if (parent->isLeftChild) {
                for (int i = 0; i < data->subarrayCount; i++) {
                    Subarray* sub = &data->subarrays[i];
                    if (sub->isActive && sub->size > 1 && 
                        sub->level == parent->level && 
                        sub->parentIndex == parent->parentIndex &&
                        !sub->isLeftChild) {
                        // Found right sibling that needs processing
                        return i;
                    }
                }
            }
            
            // No right sibling or parent is right child, continue up the stack
            return FindNextFocusAfterMerge(data, parent->parentIndex);
        }
    }
    
    // Stack is empty or invalid, look for any remaining subarrays that need splitting
    // Start from highest level and work down
    for (int level = data->maxLevel; level >= 1; level--) {
        for (int i = 0; i < data->subarrayCount; i++) {
            Subarray* sub = &data->subarrays[i];
            if (sub->isActive && sub->size > 1 && sub->level == level) {
                return i;
            }
        }
    }
    
    return -1; // No more subarrays to process
}

static void PrintMergeSortState(MergeSortData* data) {
    printf("=== MERGE SORT STATE ===\n");
    printf("Phase: %s\n", 
           data->currentPhase == PHASE_SPLITTING ? "SPLITTING" :
           data->currentPhase == PHASE_MERGING ? "MERGING" : "COMPLETE");
    printf("Current Focus: %d\n", data->currentFocusIndex);
    printf("Stack Size: %d\n", data->focusStackSize);
    printf("Subarrays (%d total):\n", data->subarrayCount);
    
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (sub->isActive) {
            printf("  [%d] Level %d, Size %d, %s child, Parent %d, Elements: [", 
                   i, sub->level, sub->size, 
                   sub->isLeftChild ? "Left" : "Right", 
                   sub->parentIndex);
            for (int j = 0; j < sub->size; j++) {
                printf("%d", sub->elements[j]);
                if (j < sub->size - 1) printf(",");
            }
            printf("]\n");
        }
    }
    printf("========================\n");
}