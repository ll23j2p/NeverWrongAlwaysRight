#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"

void Game1_Init(void);
void Game1_Reset(void);
void Game1_HandleInput(void);
void Game1_UpdatePlayerLane(void);
void Game1_UpdateWorldScroll(void);
void Game1_SpawnSegment(void);

MenuState Game1_Run(void);

#endif // GAME_1_H