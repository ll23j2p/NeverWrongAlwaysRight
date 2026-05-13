#ifndef GAME1_LOGIC_H
#define GAME1_LOGIC_H

// Game1_Logic.h -- game state variables and all update/input/collision prototypes.
//
// Key concepts used in this module:
// - World coordinate system: obstacles move in world space, projected to screen each frame
// - Perspective projection: linear interpolation gives depth illusion without 3D maths
// - lane_move_ready flag: prevents repeated lane switches while the joystick is held
// - Separation of logic from rendering: nothing in this file touches the LCD

#include <stdint.h>
#include "Game1_Config.h"
#include "Joystick.h"
#include "Menu.h"

// ===== GAME STATE =====
// Declared extern so Game1_Render.c and Game_1.c can read them.
// Defined once in Game1_Logic.c -- never define them again elsewhere.
extern uint32_t animation_counter;
extern int16_t  moving_x;
extern uint8_t  exit_requested;
extern int8_t   player_lane;
extern uint8_t  lane_move_ready;
extern uint16_t score;
extern uint8_t  lives;
extern uint8_t  game_over;
extern int16_t  obstacle_speed;

// obstacle arrays -- indexed by slot (0 to OBSTACLE_COUNT-1)
extern uint8_t  obstacle_active[OBSTACLE_COUNT];
extern int16_t  obstacle_world_y[OBSTACLE_COUNT];
extern int8_t   obstacle_lane[OBSTACLE_COUNT];
extern int8_t   next_spawn_lane;

// joystick is essecially shared with main.c (defined there) and read here each frame
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t     joystick_data;
extern UserInput      joystick_input;

// ===== FUNCTION PROTOTYPES =====
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
    int8_t   lane,
    int16_t  world_y,
    int16_t *screen_x,
    int16_t *screen_y,
    int16_t *size
);

#endif // GAME1_LOGIC_H
