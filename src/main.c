#include "raylib.h"
#include "core/game.h"
#include "utils/colors.h"
#include "ui/ui.h"
#include "algorithms/algorithm.h"

int main(void) {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Discretia - Algorithm Visualizer Platformer");
    SetTargetFPS(60);
    
    // Initialize game systems
    InitGame();
    InitUI();
    InitAlgorithms();
    
    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        UpdateGame();
        UpdateUI();
        
        // Render
        BeginDrawing();
        ClearBackground(UI_BACKGROUND);  // Solid white background
        
        RenderGame();
        RenderUI();
        
        EndDrawing();
    }
    
    // Cleanup
    CleanupGame();
    CleanupUI();
    CloseWindow();
    
    return 0;
}