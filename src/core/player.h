#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

// Player movement constants
#define GRAVITY 0.6f
#define JUMP_FORCE -12.0f
#define MOVE_SPEED 5.0f
#define GROUND_TOLERANCE 10.0f

// Player movement and physics functions
void UpdatePlayerMovement(GameData* game);
void RenderPlayer(GameData* game);

#endif // PLAYER_H