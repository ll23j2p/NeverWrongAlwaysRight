// Game_1.c -- main game loop entry point for Game 1.
//
// Key concepts used in this file:
// - Game loop pattern: HANDLE INPUT -> UPDATE OBJECTS -> RENDER (full clear/redraw each frame)
// - Fixed timestep: HAL_GetTick() caps each iteration at GAME1_FRAME_TIME_MS
// - PWM brightness tied to player X position -- hardware reacts to in-game state
// - Buzzer plays a startup tone on launch; collision tones are fired in Game1_CheckCollisions
//
// Controls:
//   Joystick LEFT/RIGHT -- change lane
//   BT3                 -- return to main menu (or restart after game over)
//
// Architecture (mirrors the Pong lab):
//   Game1_Config.h  -- constants
//   Game1_Logic.c   -- state, input, update, collision, projection
//   Game1_Render.c  -- all drawing
//   Game_1.c        -- game loop (this file)

#include "Game_1.h"
#include "Game1_Logic.h"
#include "Game1_Render.h"
#include "Game1_Config.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"

extern PWM_cfg_t    pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;

// main entry point -- called by the menu system when the player selects Game 1
// returns MENU_STATE_HOME when BT3 is pressed to exit back to the menu
MenuState Game1_Run(void)
{
    uint32_t frame_start;
    uint32_t frame_time;

    // initialise all game state and peripherals before entering the loop
    Game1_Init();

    // startup tone to signal game has launched
    buzzer_tone(&buzzer_cfg, 1000, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);

    while (1) {
        frame_start = HAL_GetTick();

        // Step 1: handle input
        Game1_HandleInput();

        if (exit_requested) {
            PWM_SetDuty(&pwm_cfg, 50); // restore neutral brightness on exit
            return MENU_STATE_HOME;
        }

        // Next step: update game state (skipped while game over screen is showing)
        if (!game_over) {
            Game1_UpdatePlayerLane();
            Game1_UpdateWorldScroll();

            if ((animation_counter % 35) == 0) {
                Game1_SpawnSegment(); // spawn a new obstacle every 35 frames
            }

            Game1_UpdateSegments();
            Game1_CheckCollisions();
            Game1_UpdateScoreSpeed();

            // PWM brightness reflects horizontal position -- left = dim, right = bright
            PWM_SetDuty(&pwm_cfg, (moving_x * 100) / 240);
        }

        // Step 3: render full frame (clear and redraw every frame for simplicity)
        Game1_RenderFrame();

        // cap frame time to maintain consistent GAME1_FRAME_TIME_MS per iteration
        frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }
}
