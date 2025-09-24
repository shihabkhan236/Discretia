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
    
    // Track which elements are being worked on
    bool* originalActive;  // Which elements in original array are still active
    
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

void MergeSortUpdate(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return;
    
    // Physics collision is now handled in player.c via MergeSortGetPlayerPlatform
    int playerPlatform = GetPlayerPlatform(game);
    
    // Handle input based on current phase
    if (data->currentPhase == PHASE_SPLITTING) {
        // Handle G key for splitting arrays
        if (IsKeyPressed(KEY_G)) {
            if (playerPlatform >= 0 && data->subarrayCount == 0) {
                // First split - create initial subarrays from original array
                SplitArray(data, -1, game); // -1 means split original array
                UpdateAllPlatforms(data, game);
                printf("Created initial split from original array\n");
                return;
            }
            
            // Check if player is on any subarray for splitting
            for (int i = 0; i < data->subarrayCount; i++) {
                Subarray* sub = &data->subarrays[i];
                if (!sub->isActive || !sub->canSplit) continue;
                
                // Check if player is on this subarray's platforms
                for (int j = 0; j < sub->size; j++) {
                    Rectangle platform = sub->platforms[j];
                    if (game->player.x + game->player.width > platform.x + 8 && 
                        game->player.x < platform.x + platform.width - 8 &&
                        abs((int)(game->player.y + game->player.height - platform.y)) < GROUND_TOLERANCE) {
                        
                        SplitArray(data, i, game);
                        UpdateAllPlatforms(data, game);
                        
                        // Check if recursion is complete (transition to merge phase)
                        if (data->recursionComplete) {
                            data->currentPhase = PHASE_MERGING;
                            StartMergePhase(data);
                            printf("Transitioning to merge phase\n");
                        }
                        return;
                    }
                }
            }
        }
    } else if (data->currentPhase == PHASE_MERGING) {
        // Update merge highlights
        UpdateMergeHighlights(data);
        
        // Handle F key for merging elements
        if (IsKeyPressed(KEY_F)) {
            // Check if player is on original array platform
            if (playerPlatform >= 0) {
                printf("Cannot select from original array during merge phase\n");
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
                        
                        // Process merge choice
                        if (ProcessMergeChoice(game, data, i, j)) {
                            printf("Correct merge choice!\n");
                        } else {
                            printf("Incorrect merge choice - lost a heart!\n");
                        }
                        return;
                    }
                }
            }
            
            printf("Not standing on any mergeable element\n");
        }
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
    if (game->gameComplete && data->currentPhase == PHASE_COMPLETE) {
        ChangeState(STATE_LEVEL_COMPLETE);
    }
}

void MergeSortRender(GameData* game) {
    MergeSortData* data = (MergeSortData*)game->algorithmData;
    if (!data) return;
    
    // The original array is rendered by game.c - we just render additional subarrays
    
    // Render all active subarrays (positioned above original array)
    for (int i = 0; i < data->subarrayCount; i++) {
        Subarray* sub = &data->subarrays[i];
        if (!sub->isActive) continue;
        
        // Draw subarray platforms and elements with same style as original
        for (int j = 0; j < sub->size; j++) {
            Rectangle platform = sub->platforms[j];
            
            // Check if this element should be highlighted
            bool isHighlighted = (data->highlightingEnabled && 
                                i == data->highlightedSubarray && 
                                j == data->highlightedElement);
            
            // Use highlighting color if this element is highlighted
            Color platformColor = isHighlighted ? YELLOW : UI_BUTTON_NORMAL;
            Color borderColor = isHighlighted ? ORANGE : LIGHTGRAY;
            
            DrawRectangleRec(platform, platformColor);
            DrawRectangleLinesEx(platform, 2, borderColor);
            
            // Draw element number with same style as original (0 means empty box)
            if (sub->elements[j] != 0) {
                char numText[8];
                sprintf(numText, "%d", sub->elements[j]);
                Color textColor = isHighlighted ? BLACK : UI_TEXT_PRIMARY;
                DrawCenteredText(numText, 
                               platform.x + platform.width/2, 
                               platform.y + platform.height/2, 
                               FONT_SIZE_BUTTON * 1.5, textColor);
            } else {
                // Draw empty box indicator
                Color emptyColor = isHighlighted ? DARKGRAY : GRAY;
                DrawText("□", 
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
                phaseText = "MERGING PHASE - Press F on highlighted element (choose smaller value)";
            } else {
                phaseText = "MERGING PHASE - All merges complete!";
            }
            break;
        case PHASE_COMPLETE:
            phaseText = "MERGE SORT COMPLETE!";
            break;
    }
    
    // DrawText(phaseText, 20, 120, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
    
    // // Draw statistics
    // char statsText[128];
    // sprintf(statsText, "Splits: %d | Merges: %d | Max Level: %d | Recursion: %s", 
    //         data->splitCount, data->mergeCount, data->maxLevel, 
    //         data->recursionComplete ? "Complete" : "In Progress");
    // DrawText(statsText, 20, 140, FONT_SIZE_SMALL, UI_TEXT_PRIMARY);
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
        printf("Initial split of original array into 2 subarrays\n");
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
    
    // Add new subarrays
    AddSubarray(data, left);
    AddSubarray(data, right);
    
    // Update max level
    if (newLevel > data->maxLevel) {
        data->maxLevel = newLevel;
    }
    
    data->splitCount++;
    printf("Split subarray at level %d, now have %d active subarrays\n", newLevel, data->subarrayCount);
    
    // Check if recursion is complete (all subarrays are single elements)
    bool allSingleElements = true;
    for (int i = 0; i < data->subarrayCount; i++) {
        if (data->subarrays[i].isActive && data->subarrays[i].size > 1) {
            allSingleElements = false;
            break;
        }
    }
    
    if (allSingleElements) {
        data->recursionComplete = true;
        printf("Recursion complete - all subarrays are single elements\n");
    }
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
    
    // Highlight the next elements to be compared
    if (data->leftMergeIndex >= 0 && data->rightMergeIndex >= 0) {
        Subarray* left = &data->subarrays[data->leftMergeIndex];
        Subarray* right = &data->subarrays[data->rightMergeIndex];
        
        // Determine which element should be highlighted (smaller one)
        if (data->leftPointer < left->size && data->rightPointer < right->size) {
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