#include "algorithm.h"
#include "../core/game.h"
#include "../utils/colors.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Forward declarations for algorithm implementations
// Bubble Sort
void BubbleSortInit(GameData* game);
void BubbleSortUpdate(GameData* game);
void BubbleSortRender(GameData* game);
void BubbleSortCleanup(GameData* game);
bool BubbleSortIsComplete(GameData* game);
void BubbleSortResetLevel(GameData* game, int level);

// Selection Sort
void SelectionSortInit(GameData* game);
void SelectionSortUpdate(GameData* game);
void SelectionSortRender(GameData* game);
void SelectionSortCleanup(GameData* game);
bool SelectionSortIsComplete(GameData* game);
void SelectionSortResetLevel(GameData* game, int level);

// Insertion Sort
void InsertionSortInit(GameData* game);
void InsertionSortUpdate(GameData* game);
void InsertionSortRender(GameData* game);
void InsertionSortCleanup(GameData* game);
bool InsertionSortIsComplete(GameData* game);
void InsertionSortResetLevel(GameData* game, int level);

// Merge Sort
void MergeSortInit(GameData* game);
void MergeSortUpdate(GameData* game);
void MergeSortRender(GameData* game);
void MergeSortCleanup(GameData* game);
bool MergeSortIsComplete(GameData* game);
void MergeSortResetLevel(GameData* game, int level);

// Quick Sort
void QuickSortInit(GameData* game);
void QuickSortUpdate(GameData* game);
void QuickSortRender(GameData* game);
void QuickSortCleanup(GameData* game);
bool QuickSortIsComplete(GameData* game);
void QuickSortResetLevel(GameData* game, int level);

// Algorithm registry
AlgorithmFunctions algorithmRegistry[MAX_ALGORITHMS] = {
    // Bubble Sort
    {
        .init = BubbleSortInit,
        .update = BubbleSortUpdate,
        .render = BubbleSortRender,
        .cleanup = BubbleSortCleanup,
        .isComplete = BubbleSortIsComplete,
        .resetLevel = BubbleSortResetLevel
    },
    // Selection Sort
    {
        .init = SelectionSortInit,
        .update = SelectionSortUpdate,
        .render = SelectionSortRender,
        .cleanup = SelectionSortCleanup,
        .isComplete = SelectionSortIsComplete,
        .resetLevel = SelectionSortResetLevel
    },
    // Insertion Sort
    {
        .init = InsertionSortInit,
        .update = InsertionSortUpdate,
        .render = InsertionSortRender,
        .cleanup = InsertionSortCleanup,
        .isComplete = InsertionSortIsComplete,
        .resetLevel = InsertionSortResetLevel
    },
    // Merge Sort
    {
        .init = MergeSortInit,
        .update = MergeSortUpdate,
        .render = MergeSortRender,
        .cleanup = MergeSortCleanup,
        .isComplete = MergeSortIsComplete,
        .resetLevel = MergeSortResetLevel
    },
    // Quick Sort
    {
        .init = QuickSortInit,
        .update = QuickSortUpdate,
        .render = QuickSortRender,
        .cleanup = QuickSortCleanup,
        .isComplete = QuickSortIsComplete,
        .resetLevel = QuickSortResetLevel
    }
};

// Algorithm names
const char* algorithmNames[MAX_ALGORITHMS] = {
    "Bubble Sort",
    "Selection Sort", 
    "Insertion Sort",
    "Merge Sort",
    "Quick Sort"
};

void InitAlgorithms(void) {
    srand((unsigned int)time(NULL));
    printf("Algorithms initialized\n");
}

AlgorithmFunctions* GetAlgorithm(AlgorithmType type) {
    if (type >= 0 && type < MAX_ALGORITHMS) {
        return &algorithmRegistry[type];
    }
    return NULL;
}

void ShuffleArray(int* array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void GenerateRandomArray(int* array, int size, int minVal, int maxVal) {
    for (int i = 0; i < size; i++) {
        array[i] = minVal + rand() % (maxVal - minVal + 1);
    }
}

bool IsArraySorted(int* array, int size, bool ascending) {
    for (int i = 0; i < size - 1; i++) {
        if (ascending) {
            if (array[i] > array[i + 1]) return false;
        } else {
            if (array[i] < array[i + 1]) return false;
        }
    }
    return true;
}

// Algorithm implementations are in separate files:
// - bubble_sort.c
// - selection_sort.c  
// - insertion_sort.c
// - merge_sort.c
// - quick_sort.c