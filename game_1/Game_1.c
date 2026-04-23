#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;

/**
 * @brief Game 1 Implementation - Student can modify
 *
 * EXAMPLE: Shows how to use PWM LED for visual feedback
 * This is a placeholder with a lane-based animation.
 * Replace this with your actual game logic!
 */

// Game state - customize for your game
static uint32_t animation_counter = 0;
static int16_t moving_x = 100;
static int8_t move_direction = 1;
static uint8_t exit_requested = 0;
static int8_t player_lane = 1;

static const int16_t lane_x[3] = {40, 100, 160};

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

void Game1_Init(void) {
    animation_counter = 0;
    moving_x = 100;
    move_direction = 1;
    exit_requested = 0;
    player_lane = 1;
}

void Game1_Reset(void) {
    animation_counter = 0;
    moving_x = 100;
    move_direction = 1;
    exit_requested = 0;
    player_lane = 1;
}

void Game1_HandleInput(void) {
    Input_Read();

    if (current_input.btn3_pressed) {
        exit_requested = 1;
    }
}

void Game1_UpdatePlayerLane(void) {
    if ((animation_counter % 40) == 0) {
        if (player_lane >= 2) {
            move_direction = -1;
        }
        else if (player_lane <= 0) {
            move_direction = 1;
        }

        player_lane += move_direction;
    }

    if (moving_x < lane_x[player_lane]) {
        moving_x += 5;
        if (moving_x > lane_x[player_lane]) {
            moving_x = lane_x[player_lane];
        }
    }
    else if (moving_x > lane_x[player_lane]) {
        moving_x -= 5;
        if (moving_x < lane_x[player_lane]) {
            moving_x = lane_x[player_lane];
        }
    }
}

MenuState Game1_Run(void) {
    Game1_Init();
   
    buzzer_tone(&buzzer_cfg, 1000, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);
   
    MenuState exit_state = MENU_STATE_HOME;
   
    while (1) {
        uint32_t frame_start = HAL_GetTick();
       
        Game1_HandleInput();
       
        if (exit_requested) {
            PWM_SetDuty(&pwm_cfg, 50);
            exit_state = MENU_STATE_HOME;
            break;
        }
       
        animation_counter++;
        Game1_UpdatePlayerLane();
       
        {
            uint8_t brightness = (moving_x * 100) / 200;
            PWM_SetDuty(&pwm_cfg, brightness);
        }
       
        LCD_Fill_Buffer(0);
       
        LCD_printString("GAME 1", 60, 10, 1, 3);

        LCD_printString("|", 40, 80, 1, 2);
        LCD_printString("|", 100, 80, 1, 2);
        LCD_printString("|", 160, 80, 1, 2);

        LCD_printString("[*]", moving_x, 100, 1, 3);
       
        {
            char counter[32];
            sprintf(counter, "Frame: %lu", animation_counter);
            LCD_printString(counter, 50, 150, 1, 2);
        }

        {
            char lane_str[32];
            sprintf(lane_str, "Lane: %d", player_lane);
            LCD_printString(lane_str, 50, 165, 1, 2);
        }
       
        LCD_printString("Lane Update Demo", 25, 180, 1, 1);
        LCD_printString("Press BT3 to", 40, 220, 1, 1);
        LCD_printString("Return to Menu", 40, 235, 1, 1);
       
        LCD_Refresh(&cfg0);
       
        {
            uint32_t frame_time = HAL_GetTick() - frame_start;
            if (frame_time < GAME1_FRAME_TIME_MS) {
                HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
            }
        }
    }
   
    return exit_state;
}