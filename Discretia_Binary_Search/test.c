#include <stdio.h>
#include <math.h>
#include "Raylib_include/raylib.h"

#define WINDOW_WIDTH 2000
#define WINDOW_HEIGHT 1300

Color bgCOLOR = {0, 0, 0, 255};

typedef struct{
    float posx;
    float posy;
    float len;
    float velx;
    float vely;
    float accx;
    float accy;
}Player;


int main(void)
{

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Test");
    InitAudioDevice();

    /* Initialize GameObjects */
    Player square = {500, 500, 15, 0, 0, 0, 0};

    float r = 150, theta = 0;
    

    SetTargetFPS(120);
    int frame = 0;
    while (!WindowShouldClose())
    {
        
        
        /* Generation (e.g. Enemy/Collectibles/LevelLookAhead */
        
        
        
        /* Input Handling */


        

        /* Calculations (e.g. Movement/Collection/WinCondition/Death) */
        square.velx += square.accx;
        square.vely += square.accy;
        
        square.posx += square.velx;
        square.posy += square.vely;
        //r += .2;
        theta += PI / 500;
        square.posx = r * cos(theta) + 500;
        square.posy = sin(r * cos(theta)) + 500;
        
        /* Render */
        BeginDrawing();
            //ClearBackground(bgCOLOR);
            DrawRectangle(square.posx, square.posy, square.len, square.len, YELLOW);

        EndDrawing();

        frame++;
    }

    /* Uninitialize Gameobjects */

    CloseAudioDevice();
    CloseWindow();

    return 0;
}