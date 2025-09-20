#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include "../core/game.h"
#include "raylib.h"

// Merge Sort constants
#define MAX_SUBARRAYS 32
#define LEVEL_HEIGHT 120
#define FADE_SPEED 2.0f

// Merge Sort phases
typedef enum {
    PHASE_SPLITTING,
    PHASE_MERGING,
    PHASE_COMPLETE
} MergeSortPhase;

// Subarray structure for merge sort tree
typedef struct {
    int* elements;           // Array elements
    int size;               // Number of elements
    int capacity;           // Allocated capacity
    Vector2 position;       // Screen position
    Rectangle* platforms;   // Platform rectangles for this subarray
    int level;             // Recursion level (0 = base, 1 = first split, etc.)
    int index;             // Index within level (left=0, right=1, etc.)
    bool isActive;         // Whether subarray is currently visible
    bool isExhausted;      // Whether all elements have been merged
    float fadeAlpha;       // For vanish animation (1.0 = visible, 0.0 = gone)
} Subarray;

// Merge Sort data structure
typedef struct {
    Subarray* subarrays;    // Dynamic array of all subarrays
    int subarrayCount;      // Number of active subarrays
    int subarrayCapacity;   // Allocated capacity
    
    // Game state
    MergeSortPhase currentPhase;
    int activeLevel;        // Current level player is working on
    int maxLevel;          // Highest level reached
    
    // Merge state
    int leftSubarrayIndex;  // Index of left subarray being merged
    int rightSubarrayIndex; // Index of right subarray being merged
    int targetSubarrayIndex; // Index of target subarray for merge
    int leftPointer;       // Current position in left subarray
    int rightPointer;      // Current position in right subarray
    int targetPointer;     // Current position in target subarray
    
    // Camera system
    Camera2D camera;       // Camera for following player
    Vector2 cameraTarget;  // Smooth camera target
    
    // Statistics
    int splitCount;
    int mergeCount;
    int mistakes;
    
    // Animation
    float fadeTimer;       // For subarray vanish animations
} MergeSortData;

// Merge Sort function declarations
void MergeSortInit(GameData* game);
void MergeSortUpdate(GameData* game);
void MergeSortRender(GameData* game);
void MergeSortCleanup(GameData* game);
bool MergeSortIsComplete(GameData* game);
void MergeSortResetLevel(GameData* game, int level);

// Subarray management functions
Subarray* CreateSubarray(int* elements, int size, int level, int index);
void AddSubarray(MergeSortData* data, Subarray subarray);
void RemoveSubarray(MergeSortData* data, int index);
void FreeSubarray(Subarray* subarray);
void ClearSubarrayElements(Subarray* subarray);

// Position and rendering functions
Vector2 CalculateSubarrayPosition(int level, int index, int totalAtLevel);
void UpdateSubarrayPositions(MergeSortData* data);
void RenderSubarray(Subarray* subarray, GameData* game);

// Merge sort specific functions
void SplitArray(MergeSortData* data, int subarrayIndex);
void StartMergePhase(MergeSortData* data);
bool ProcessMergeChoice(MergeSortData* data, int chosenElement);
void VanishSubarray(MergeSortData* data, int subarrayIndex);
bool ValidateMergeChoice(MergeSortData* data, int chosenElement);

// Camera functions
void UpdateCameraPlayerBoundsPush(Camera2D* camera, GameData* game, MergeSortData* data, float deltaTime);

#endif // MERGE_SORT_H