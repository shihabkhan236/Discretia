#include "src/core/game.h"
#include "raylib.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Discretia - Algorithm Visualizer");
    SetTargetFPS(60);

    InitGame();

    while (!WindowShouldClose()) {
        UpdateGame();

        BeginDrawing();
        ClearBackground(WHITE);
        RenderGame();
        EndDrawing();
    }

    CleanupGame();
    CloseWindow();
    return 0;
}
