#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include "raylib_include/raylib.h"


#define WINDOW_WIDTH 2000
#define WINDOW_HEIGHT 1300

int SORT_ORDER = 1;

int PLAT_LAST = 7;
int PLAT_START = 0;
int PLAT_X = 80;
#define PLAT_Y 750
#define PLAT_LEN 130
#define PLAT_WIDTH 75
#define PLAT_SPACING 250

#define GRAVITY 0.5f
#define JUMP_FORCE -8.8f
#define MOVE_SPEED 5.0f

Color bgCOLOR = {0, 0, 0, 255};

typedef struct{
    float posx;
    float posy;
    int len;
    int width;
    int value;
    bool existance;
    bool revealed;
}Platform;

typedef struct{
    int len;
    float posx;
    float posy;
    float velx;
    float vely;
    int Plat_index;
}Player;

int compare(const void* a, const void* b) {
    return SORT_ORDER * (*(int*)a - *(int*)b);
}

int* random_generator(int n, int min, int max, int* target_number){
    int* arr = malloc(sizeof(int) * n);

    for(int i = 0; i < n; i++){
        arr[i] = (rand() % (max - min + 1)) + min;
    }
    *target_number = arr[n / 2];
    qsort(arr, n, sizeof(int), compare);
    return arr;
}

int main(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Binary Search Parkour");
    InitAudioDevice();

    /* Initialize GameObjects */
    Player cube = {60, PLAT_X, PLAT_Y - 60, 0, 0, -1};
    Platform plat[PLAT_LAST];
    int platX = PLAT_X;
    
    srand(time(NULL));

    //ascending = 1, descending = -1
    int sort = (rand() % 2);
    if(! sort)  SORT_ORDER = sort = -1;

    //generating numbers
    int targetNumber;
    int* numbers = random_generator(7, 1, 20, &targetNumber);

    for(int i = 0; i < PLAT_LAST; i++){
        plat[i] = (Platform){platX, PLAT_Y, PLAT_LEN, PLAT_WIDTH, numbers[i], 1, 0};
        platX += PLAT_SPACING;
    }

    int searchStart = 0;
    int searchEnd = PLAT_LAST - 1;
    bool gameOver = false;
    bool gameWon = false;

    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        /* Input Handling */
        if (!gameOver && !gameWon) {
            cube.velx = 0;
            if(IsKeyDown(KEY_A)) cube.velx = -MOVE_SPEED;
            if(IsKeyDown(KEY_D)) cube.velx = MOVE_SPEED;

            if(IsKeyPressed(KEY_W) && cube.Plat_index != -1){
                cube.vely = JUMP_FORCE; // jump only if on a platform
            }

            // Reveal platform and binary search logic
            if(IsKeyPressed(KEY_F) && cube.Plat_index != -1){
                int midIndex = (searchStart + searchEnd) / 2;

                if(cube.Plat_index != midIndex){
                    printf("Wrong platform! Game Over.\n");
                    gameOver = true;
                } 
                else if(sort == 1){
                    plat[cube.Plat_index].revealed = 1;

                    if(plat[cube.Plat_index].value == targetNumber){
                        printf("You found the target number %d! You win!\n", targetNumber);
                        gameWon = true;
                    } 
                    else if(plat[cube.Plat_index].value > targetNumber){
                        for(int i = cube.Plat_index; i <= searchEnd; i++)    plat[i].revealed = 1;
                        searchEnd = cube.Plat_index - 1;
                    } 
                    else{
                        for(int i = cube.Plat_index; i >= searchStart; i--)    plat[i].revealed = 1;
                        searchStart = cube.Plat_index + 1;
                    }
                }

                else if(sort == -1){
                    plat[cube.Plat_index].revealed = 1;

                    if(plat[cube.Plat_index].value == targetNumber){
                        printf("You found the target number %d! You win!\n", targetNumber);
                        gameWon = true;
                    }
                    else if(plat[cube.Plat_index].value > targetNumber){
                        for(int i = cube.Plat_index; i >= searchStart; i--)    plat[i].revealed = 1;
                        searchStart = cube.Plat_index + 1;                        
                    } 
                    else{
                        for(int i = cube.Plat_index; i <= searchEnd; i++)    plat[i].revealed = 1;
                        searchEnd = cube.Plat_index - 1;
                    }
                }
            }
        }

        /* Calculations */
        cube.vely += GRAVITY;
        cube.posy += cube.vely;
        cube.posx += cube.velx;

        /* Platform Collision */
        cube.Plat_index = -1;

        for(int i = 0; i < PLAT_LAST; i++){
            if(!plat[i].existance) continue;

            float playerLeft = cube.posx;
            float playerRight = cube.posx + cube.len;
            float playerTop = cube.posy;
            float playerBottom = cube.posy + cube.len;

            float platLeft = plat[i].posx;
            float platRight = plat[i].posx + plat[i].len;
            float platTop = plat[i].posy;
            float platBottom = plat[i].posy + plat[i].width;

            // Top collision
            if(playerBottom >= platTop - 2 && playerBottom <= platTop + 10 &&
               playerRight > platLeft && playerLeft < platRight &&
               cube.vely >= 0){
                cube.posy = platTop - cube.len;
                cube.vely = 0;
                cube.Plat_index = i;
            }
            // Left collision
            else if(playerRight > platLeft && playerLeft < platLeft &&
                    cube.velx > 0 &&
                    playerBottom > platTop && playerTop < platBottom){
                cube.posx = platLeft - cube.len;
                cube.velx = 0;
            }
            // Right collision
            else if(playerLeft < platRight && playerRight > platRight &&
                    cube.velx < 0 &&
                    playerBottom > platTop && playerTop < platBottom){
                cube.posx = platRight;
                cube.velx = 0;
            }
        }

        /* Render */
        BeginDrawing();
            ClearBackground(bgCOLOR);

            DrawRectangle(cube.posx, cube.posy, cube.len, cube.len, YELLOW);

            for(int i = PLAT_START; i < PLAT_LAST; i++){
                if(plat[i].existance){
                    if(!plat[i].revealed)
                        DrawRectangle(plat[i].posx, plat[i].posy, plat[i].len, plat[i].width, BLUE);
                    else{
                        DrawRectangleLinesEx((Rectangle){plat[i].posx, plat[i].posy, plat[i].len, plat[i].width}, 5, BLUE);
                        char num_text[16];
                        sprintf(num_text, "%d", plat[i].value);
                        int textWidth = MeasureText(num_text, 20);
                        int textX = plat[i].posx + (plat[i].len / 2) - (textWidth / 2);
                        int textY = plat[i].posy + (plat[i].width / 2) - 10;
                        DrawText(num_text, textX, textY, 20, RED);
                    }
                }
            }

            if(sort == 1)   DrawText("|The Array is in Ascending Order|", 50, 50, 30, PURPLE);
            if(sort == -1)   DrawText("|The Array is in Descending Order|", 50, 50, 30, PURPLE);
            DrawText(TextFormat("Target: %d", targetNumber), 50, 100, 30, GREEN);
            if(gameOver) DrawText("Game Over!", 50, 150, 40, RED);
            if(gameWon) DrawText("You Win!", 50, 150, 40, GREEN);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
