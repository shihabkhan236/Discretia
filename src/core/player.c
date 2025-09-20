#include "game.h"
#include "raylib.h"
#include "player.h"
#include "../ui/ui.h"
#include <stdio.h>




void UpdatePlayerMovement(GameData* game) {
    // Horizontal movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        game->player.x -= MOVE_SPEED;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        game->player.x += MOVE_SPEED;
    }
    
    // Apply gravity
    game->velocity.y += GRAVITY;
    game->player.y += game->velocity.y;
    
    // Platform collision detection
    game->isOnGround = false;
    for (int i = 0; i < game->arraySize; i++) {
        Rectangle platform = game->platforms[i];
        
        // Check if player is landing on top of platform (MATCH WORKING DEMO EXACTLY)
        if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
            game->player.y + game->player.height + game->velocity.y >= platform.y &&
            game->player.x + game->player.width > platform.x + 8 && 
            game->player.x < platform.x + platform.width - 8) {
            
            game->player.y = platform.y - game->player.height;
            game->velocity.y = 0;
            game->isOnGround = true;
            // printf("Player landed on platform %d\n", i);
            break;
        }
    }

    
    
    // Jump input
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && game->isOnGround) {
        game->velocity.y = JUMP_FORCE;
        game->isOnGround = false;
        printf("Player jumped\n");
    }
    
    // Screen boundaries
    if (game->player.x < 0) {
        game->player.x = 0;
    }
    if (game->player.x + game->player.width > SCREEN_WIDTH) {
        game->player.x = SCREEN_WIDTH - game->player.width;
    }
    
    // Fall off screen - respawn and lose heart
    if (game->player.y > SCREEN_HEIGHT) {
        game->hearts--;
        if (game->hearts > 0) {
            // Respawn on first platform
            game->player.x = game->platforms[0].x + (game->platforms[0].width - game->player.width) / 2;
            game->player.y = game->platforms[0].y - game->player.height;
            game->velocity = (Vector2){0, 0};
            printf("Player respawned, hearts remaining: %d\n", game->hearts);
        } else {
            ChangeState(STATE_GAME_OVER);
        }
    }
}

void RenderPlayer(GameData* game) {
    // Draw player as a colored rectangle (no shadow)
    Color playerColor = (Color){200, 220, 255, 255};
    
    // Draw player
    DrawRectangleRec(game->player, playerColor);
    DrawRectangleLinesEx(game->player, 2, (Color){150, 170, 200, 255});



     // Draw the number the player is carrying (using default font)
    if (game->carrying && game->playerNumber > 0) {
        char numText[16];
        sprintf(numText, "%d", game->playerNumber);
        
        // Calculate centered position using default font
        int fontSize = 32;
        int textWidth = MeasureText(numText, fontSize);
        int textX = game->player.x + (game->player.width - textWidth) / 2;
        int textY = game->player.y + (game->player.height - fontSize) / 2;

        DrawText(numText, textX, textY, fontSize, BLACK);
        // DrawCenteredText(numText, textX, textY, fontSize, BLACK);
        
        
    }
}