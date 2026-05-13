#include "Game1_Logic.h"
#include "Game1_Config.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Buzzer.h"
#include "adc.h"
#include "stm32l4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern Buzzer_cfg_t buzzer_cfg;

// ===== JOYSTICK =====
// joystick_cfg and joystick_data are defined in main.c -- referenced here via extern
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t     joystick_data;
UserInput joystick_input; // local to game -- not shared with main.c

// ===== GAME STATE DEFINITIONS =====
// These are the single definitions -- all other files access them via extern in Game1_Logic.h
uint32_t animation_counter = 0;
int16_t  moving_x          = 120;
uint8_t  exit_requested    = 0;
int8_t   player_lane       = 1;
uint8_t  lane_move_ready   = 1;

uint16_t score          = 0;
uint8_t  lives          = 3;
uint8_t  game_over      = 0;
int16_t  obstacle_speed = 4;

uint8_t  obstacle_active[OBSTACLE_COUNT]  = {0, 0, 0, 0};
int16_t  obstacle_world_y[OBSTACLE_COUNT] = {0, 0, 0, 0};
int8_t   obstacle_lane[OBSTACLE_COUNT]    = {0, 0, 0, 0};
int8_t   next_spawn_lane = 0;

/* ---------- init / reset ---------- */

// resets all game state and re-initialises the joystick ready for a new game
void Game1_Init(void)
{
    int i;

    animation_counter = 0;
    moving_x          = SCREEN_CENTRE_X;
    exit_requested    = 0;
    player_lane       = 1;
    lane_move_ready   = 1;
    score             = 0;
    lives             = 3;
    game_over         = 0;
    obstacle_speed    = 4;
    next_spawn_lane   = 0;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        obstacle_active[i]  = 0;
        obstacle_world_y[i] = 0;
        obstacle_lane[i]    = 0;
    }

    Joystick_Init(&joystick_cfg);
    Joystick_Calibrate(&joystick_cfg);
}

// wraps Game1_Init -- called when the player restarts after game over
void Game1_Reset(void)
{
    Game1_Init();
}

/* ---------- input ---------- */

// reads buttons and joystick each frame
// on game over only BT3 is checked (triggers reset); during play BT3 exits to menu
void Game1_HandleInput(void)
{
    Input_Read();

    Joystick_Read(&joystick_cfg, &joystick_data);
    joystick_input = Joystick_GetInput(&joystick_data);

    if (game_over) {
        if (current_input.btn3_pressed) Game1_Reset();
        return;
    }

    if (current_input.btn3_pressed) exit_requested = 1;
}

/* ---------- player update ---------- */

// update lane from joystick and slides moving_x toward the target at 6px per frame
// lane_move_ready prevents repeated switches while the joystick stays pushed
void Game1_UpdatePlayerLane(void)
{
    int16_t target_x;

    if (joystick_input.direction == CENTRE) lane_move_ready = 1;

    if (lane_move_ready) {
        if (joystick_input.direction == W  ||
            joystick_input.direction == NW ||
            joystick_input.direction == SW) {
            if (player_lane > 0) player_lane--;
            lane_move_ready = 0;
        }
        else if (joystick_input.direction == E  ||
                 joystick_input.direction == NE ||
                 joystick_input.direction == SE) {
            if (player_lane < 2) player_lane++;
            lane_move_ready = 0;
        }
    }

    // map lane index to target screen X
    if      (player_lane == 0) target_x = SCREEN_CENTRE_X - NEAR_LANE_SPACING;
    else if (player_lane == 1) target_x = SCREEN_CENTRE_X;
    else                       target_x = SCREEN_CENTRE_X + NEAR_LANE_SPACING;

    // smooth slide toward target
    if (moving_x < target_x) {
        moving_x += 6;
        if (moving_x > target_x) moving_x = target_x;
    }
    else if (moving_x > target_x) {
        moving_x -= 6;
        if (moving_x < target_x) moving_x = target_x;
    }
}

/* ---------- world scroll ---------- */

// advances the frame counter and increases difficulty every 200 frames
// speed caps at 8 so the game stays playable
void Game1_UpdateWorldScroll(void)
{
    animation_counter++;

    if ((animation_counter % 200) == 0) {
        if (obstacle_speed < 8) obstacle_speed++;
    }
}

/* ---------- obstacle spawning ---------- */

// spawns into the first free slot
void Game1_SpawnSegment(void)
{
    int i;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i] == 0) {
            obstacle_active[i]  = 1;
            obstacle_world_y[i] = 0;        // start at far end of track
            obstacle_lane[i]    = next_spawn_lane;

            next_spawn_lane++;
            if (next_spawn_lane > 2) next_spawn_lane = 0;

            break; // only spawn one per call
        }
    }
}

/* ---------- obstacle update ---------- */

// moves each active obstacle toward the player by obstacle_speed world units
// deactivates it and awards points when it passes the near edge
void Game1_UpdateSegments(void)
{
    int i;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i]) {
            obstacle_world_y[i] += obstacle_speed;

            if (obstacle_world_y[i] > WORLD_MAX_Y) {
                obstacle_active[i]  = 0;
                obstacle_world_y[i] = 0;
                score += 10; // player successfully dodged -- reward points
            }
        }
    }
}

/* ---------- collision detection ---------- */

// checks each active obstacle in the player's lane against the collision band
// on hit: removes obstacle, decrements lives, fires buzzer, sets an interestng  game_over displayed if lives reach 0
void Game1_CheckCollisions(void)
{
    int i;
    int16_t screen_x, screen_y, size;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i] && obstacle_lane[i] == player_lane) {

            Game1_ProjectToScreen(
                obstacle_lane[i], obstacle_world_y[i],
                &screen_x, &screen_y, &size
            );

            if (screen_y >= PLAYER_COLLISION_Y_MIN &&
                screen_y <= PLAYER_COLLISION_Y_MAX) {

                obstacle_active[i]  = 0;
                obstacle_world_y[i] = 0;

                if (lives > 0) lives--;

                buzzer_tone(&buzzer_cfg, 1500, 40); // short collision tone
                HAL_Delay(30);
                buzzer_off(&buzzer_cfg);

                if (lives == 0) game_over = 1;
            }
        }
    }
}

// increments score by 1 each active frame -- rewards survival over time
void Game1_UpdateScoreSpeed(void)
{
    if (!game_over) score++;
}

/* ---------- perspective projection ---------- */

// project a world position (lane, world_y) to screen coardinates and sprite size
// uses linear interpolation for all three outputs to simulate perspective depth:
//   screen_y:     HORIZON_Y         -> NEAR_Y              (top to bottom)
//   lane_spacing: FAR_LANE_SPACING  -> NEAR_LANE_SPACING   (narrow to wide)
//   size:         OBSTACLE_MIN_SIZE -> OBSTACLE_MAX_SIZE    (small to large)
void Game1_ProjectToScreen(
    int8_t   lane,
    int16_t  world_y,
    int16_t *screen_x,
    int16_t *screen_y,
    int16_t *size
)
{
    int16_t lane_spacing = FAR_LANE_SPACING +
                           ((world_y * (NEAR_LANE_SPACING - FAR_LANE_SPACING)) / WORLD_MAX_Y);

    *screen_y = HORIZON_Y + ((world_y * (NEAR_Y        - HORIZON_Y))        / WORLD_MAX_Y);
    *screen_x = SCREEN_CENTRE_X + ((lane - 1) * lane_spacing);
    *size     = OBSTACLE_MIN_SIZE +
                ((world_y * (OBSTACLE_MAX_SIZE - OBSTACLE_MIN_SIZE)) / WORLD_MAX_Y);
}
