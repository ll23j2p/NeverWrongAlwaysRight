#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"
#include <stdint.h>

void Game1_Init(void);
void Game1_Reset(void);
void Game1_HandleInput(void);
void Game1_UpdatePlayerLane(void);
void Game1_UpdateWorldScroll(void);
void Game1_SpawnSegment(void);
void Game1_UpdateSegments(void);
void Game1_CheckCollisions(void);
void Game1_UpdateScoreSpeed(void);

void Game1_ProjectToScreen(
    int8_t lane,
    int16_t world_y,
    int16_t *screen_x,
    int16_t *screen_y,
    int16_t *size
);

void Game1_DrawTrack(void);
void Game1_DrawObstacleSpriteAt(int16_t centre_x, int16_t centre_y, int16_t target_size);
void Game1_DrawEntities(void);
void Game1_DrawHUD(void);
void Game1_RenderFrame(void);

MenuState Game1_Run(void);

#endif // GAME_1_H