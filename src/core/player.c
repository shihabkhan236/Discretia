#include "game.h"
#include "raylib.h"
#include <stdio.h>

// Player movement constants
#define GRAVITY 0.6f
#define JUMP_FORCE -12.0f
#define MOVE_SPEED 5.0f
#define GROUND_TOLERANCE 10.0f

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
        
        // Check if player is landing on top of platform
        if (game->player.y + game->player.height <= platform.y + GROUND_TOLERANCE &&
            game->player.y + game->player.height + game->velocity.y >= platform.y &&
            // game->player.x + game->player.width > platform.x && 
            // game->player.x < platform.x + platform.width
            // OLD (too wide):
            // game->player.x + game->player.width > platform.x && 
            // game->player.x < platform.x + platform.width
            // NEW (matches demo):
            game->player.x + game->player.width > platform.x + 8 && 
            game->player.x < platform.x + platform.width - 8
            ) {
            
            game->player.y = platform.y - game->player.height;
            game->velocity.y = 0;
            game->isOnGround = true;
            break;
        }
    }
    
    // Jump input
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && game->isOnGround) {
        game->velocity.y = JUMP_FORCE;
        game->isOnGround = false;
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
}