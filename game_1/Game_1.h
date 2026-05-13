#ifndef GAME_1_H
#define GAME_1_H

// Game_1.h -- master header for Game 1.
// Includes this single header anywhere that needs to launch Game 1.
//
// File structure inspired by labs and espacially aftear analysing at the Pong game lab
//   Game1_Config.h  -- all #define constants and palette indices
//   Game1_Logic.h/c -- game state, input, update, collision, projection
//   Game1_Render.h/c -- all drawing and sprite functions
//   Game_1.c        -- game loop entry point (equivalent to main.c in Pong)

#include "Game1_Config.h"
#include "Game1_Logic.h"
#include "Game1_Render.h"
#include "Menu.h"

MenuState Game1_Run(void);

#endif // GAME_1_H
