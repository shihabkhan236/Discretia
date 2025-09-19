#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "../core/game.h"
#include <stdbool.h>

// Algorithm function pointers
typedef struct {
    void (*init)(GameData* game);
    void (*update)(GameData* game);
    void (*render)(GameData* game);
    void (*cleanup)(GameData* game);
    bool (*isComplete)(GameData* game);
    void (*resetLevel)(GameData* game, int level);
} AlgorithmFunctions;

// Algorithm registry
extern AlgorithmFunctions algorithmRegistry[MAX_ALGORITHMS];

// Algorithm names
extern const char* algorithmNames[MAX_ALGORITHMS];

// Initialize all algorithms
void InitAlgorithms(void);

// Get algorithm functions
AlgorithmFunctions* GetAlgorithm(AlgorithmType type);

// Utility functions for algorithms
void ShuffleArray(int* array, int size);
void GenerateRandomArray(int* array, int size, int minVal, int maxVal);
bool IsArraySorted(int* array, int size, bool ascending);

#endif // ALGORITHM_H