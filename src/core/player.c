#include "game.h"
#include "raylib.h"
#include "player.h"
#include "../ui/ui.h"
#include "../algorithms/merge_sort.h"
#include "../algorithms/quick_sort.h"
#include <stdio.h>

void UpdatePlayerMovement(GameData *game)
{

    // Horizontal movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    {
        game->player.x -= MOVE_SPEED;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    {
        game->player.x += MOVE_SPEED;
    }

    // Apply gravity
    game->velocity.y += GRAVITY;
    game->player.y += game->velocity.y;

    // Platform collision detection
    game->isOnGround = false;

    // Enhanced collision detection for Quick Sort algorithm
    if (game->selectedAlgorithm == ALGO_QUICK_SORT)
    {
        for (int i = 0; i < game->arraySize; i++)
        {
            Rectangle platform = game->platforms[i];
            Rectangle elevatedPlatform = platform;
            Rectangle fallenPlatform = platform;

            // Handle different platform states with priority-based collision
            if (QuickSortIsCompletedPivot(game, i))
            {
                // Adjusted collision positioning: fallen platforms move down with the visual
                fallenPlatform.y += BOX_SIZE + (BOX_SIZE / 2);

                // Check collision with fallen platform
                if (game->player.y + game->player.height <= fallenPlatform.y + GROUND_TOLERANCE &&
                    game->player.y + game->player.height + game->velocity.y >= fallenPlatform.y &&
                    game->player.x + game->player.width > fallenPlatform.x + 8 &&
                    game->player.x < fallenPlatform.x + fallenPlatform.width - 8)
                {

                    game->player.y = fallenPlatform.y - game->player.height;
                    game->velocity.y = 0;
                    game->isOnGround = true;
                    break;
                }
            }
            else if (QuickSortIsSwappingElement(game, i))
            {
                // Dual-height collision checking: check elevated position first (priority-based)
                elevatedPlatform.y -= BOX_SIZE + (BOX_SIZE / 2);

                // Priority 1: Check collision with elevated platform (if player can reach it)
                if (game->player.y + game->player.height <= elevatedPlatform.y + GROUND_TOLERANCE &&
                    game->player.y + game->player.height + game->velocity.y >= elevatedPlatform.y &&
                    game->player.x + game->player.width > elevatedPlatform.x + 8 &&
                    game->player.x < elevatedPlatform.x + elevatedPlatform.width - 8)
                {

                    game->player.y = elevatedPlatform.y - game->player.height;
                    game->velocity.y = 0;
                    game->isOnGround = true;
                    break;
                }

                // Priority 2: Check collision with original platform position (for walking under)
                if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
                    game->player.y + game->player.height + game->velocity.y >= platform.y &&
                    game->player.x + game->player.width > platform.x + 8 &&
                    game->player.x < platform.x + platform.width - 8)
                {

                    game->player.y = platform.y - game->player.height;
                    game->velocity.y = 0;
                    game->isOnGround = true;
                    break;
                }
            }
            else
            {
                // Normal platforms - standard collision
                if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
                    game->player.y + game->player.height + game->velocity.y >= platform.y &&
                    game->player.x + game->player.width > platform.x + 8 &&
                    game->player.x < platform.x + platform.width - 8)
                {

                    game->player.y = platform.y - game->player.height;
                    game->velocity.y = 0;
                    game->isOnGround = true;
                    break;
                }
            }
        }
    }
    else
    {
        // Original platform collision detection for other algorithms
        for (int i = 0; i < game->arraySize; i++)
        {
            Rectangle platform = game->platforms[i];

            // Check if player is landing on top of platform (MATCH WORKING DEMO EXACTLY)
            if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
                game->player.y + game->player.height + game->velocity.y >= platform.y &&
                game->player.x + game->player.width > platform.x + 8 &&
                game->player.x < platform.x + platform.width - 8)
            {

                game->player.y = platform.y - game->player.height;
                game->velocity.y = 0;
                game->isOnGround = true;
                break;
            }
        }
    }

    // If not on original platforms, check algorithm-specific platforms
    if (!game->isOnGround && game->selectedAlgorithm == ALGO_MERGE_SORT)
    {
        MergeSortGetPlayerPlatform(game);
    }

    // Jump input
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && game->isOnGround)
    {
        game->velocity.y = JUMP_FORCE;
        game->isOnGround = false;
    }

    // Fall off screen - respawn and lose heart
    if (game->player.y > SCREEN_HEIGHT)
    {
        game->hearts--;
        if (game->hearts > 0)
        {
            // Respawn on first platform
            game->player.x = game->platforms[0].x + (game->platforms[0].width - game->player.width) / 2;
            game->player.y = game->platforms[0].y - game->player.height;
            game->velocity = (Vector2){0, 0};
            printf("Player respawned, hearts remaining: %d\n", game->hearts);
        }
        else
        {
            ChangeState(STATE_GAME_OVER);
        }
    }
}

void RenderPlayer(GameData *game)
{
    // Draw player as a colored rectangle (no shadow)
    Color playerColor = (Color){200, 220, 255, 255};

    // Draw player
    DrawRectangleRec(game->player, playerColor);
    DrawRectangleLinesEx(game->player, 2, (Color){150, 170, 200, 255});

    // Draw the number the player is carrying (using default font)
    if (game->carrying && game->playerNumber > 0)
    {
        char numText[16];
        sprintf(numText, "%d", game->playerNumber);

        // Calculate centered position using default font
        int fontSize = 32;
        int textWidth = MeasureText(numText, fontSize);
        int textX = game->player.x + (game->player.width - textWidth) / 2;
        int textY = game->player.y + (game->player.height - fontSize) / 2;

        DrawText(numText, textX, textY, fontSize, BLACK);
    }

    // For merge sort algorithm, check if player is carrying a number
    if (game->selectedAlgorithm == ALGO_MERGE_SORT)
    {
        if (MergeSortIsCarryingNumber(game))
        {
            int carriedNumber = MergeSortGetCarriedNumber(game);
            if (carriedNumber > 0)
            {
                char numText[16];
                sprintf(numText, "%d", carriedNumber);

                // Calculate centered position using default font
                int fontSize = 32;
                int textWidth = MeasureText(numText, fontSize);
                int textX = game->player.x + (game->player.width - textWidth) / 2;
                int textY = game->player.y + (game->player.height - fontSize) / 2;

                DrawText(numText, textX, textY, fontSize, BLACK);
            }
        }
    }
}