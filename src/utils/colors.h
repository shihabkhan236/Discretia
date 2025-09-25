#ifndef COLORS_H
#define COLORS_H

#include "raylib.h"

// UI Color Palette
#define UI_BACKGROUND (Color){255, 255, 255, 255}    // white
#define UI_TEXT_PRIMARY (Color){20, 20, 20, 255}     // Near black
#define UI_BORDER (Color){100, 100, 100, 255}        // Medium gray
#define UI_BUTTON_NORMAL (Color){255, 255, 255, 255} // White
#define UI_BUTTON_HOVER (Color){230, 230, 230, 255}  // Light gray
#define UI_BUTTON_ACTIVE (Color){200, 0, 0, 255}     // Darker gray

// Game Color Palette
#define GAME_HEART (Color){220, 50, 50, 255}       // Red hearts
#define GAME_HIGHLIGHT (Color){255, 200, 0, 255}   // Yellow highlight
#define GAME_SORTED (Color){100, 200, 100, 255}    // Green for sorted
#define GAME_UNSORTED (Color){150, 150, 200, 255}  // Blue for unsorted
#define GAME_SELECTED (Color){255, 150, 50, 255}   // Orange for selected
#define GAME_COMPARING (Color){255, 100, 100, 255} // Light red for comparing

// Algorithm-specific colors
#define BUBBLE_COMPARE (Color){255, 200, 100, 255}  // Orange for bubble sort comparison
#define SELECTION_MIN (Color){100, 255, 100, 255}   // Bright green for minimum
#define INSERTION_SHIFT (Color){200, 100, 255, 255} // Purple for insertion shift
#define MERGE_DIVIDE (Color){100, 200, 255, 255}    // Light blue for merge divisions
#define QUICK_PIVOT (Color){255, 100, 200, 255}     // Pink for pivot
#define QUICK_COMPLETED (Color){144, 238, 144, 255} // Light green for completed pivots

// Utility functions
Color LerpColor(Color start, Color end, float t);
Color FadeColor(Color color, float alpha);

#endif // COLORS_H