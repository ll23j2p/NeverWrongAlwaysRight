#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include "assets/New_Piskel_indexed.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;

/**
 * @brief Game 1 Implementation - Student can modify
 *
 * Current version:
 * - player moves between 3 lanes
 * - road scrolls downward
 * - obstacles spawn in lanes
 * - uploaded sprite is now drawn as the player at 2x scale
 */

/* ===== GAME STATE ===== */
static uint32_t animation_counter = 0;
static int16_t moving_x = 100;
static int8_t move_direction = 1;
static uint8_t exit_requested = 0;
static int8_t player_lane = 1;
static int16_t scroll_y = 0;

static const int16_t lane_x[3] = {40, 100, 160};

#define OBSTACLE_COUNT 3
static uint8_t obstacle_active[OBSTACLE_COUNT] = {0, 0, 0};
static int16_t obstacle_y[OBSTACLE_COUNT] = {0, 0, 0};
static int8_t obstacle_lane[OBSTACLE_COUNT] = {0, 0, 0};
static int8_t next_spawn_lane = 0;

/* ===== GAME CONFIG ===== */
#define GAME1_FRAME_TIME_MS 30
#define PLAYER_SPRITE_SCALE 2
#define PLAYER_SPRITE_Y 180

/**
 * @brief Draw the uploaded player sprite centered on the current lane
 */
static void Game1_DrawPlayerSprite(void)
{
    int16_t sprite_width_scaled;
    int16_t sprite_x;

    sprite_width_scaled = NEW_PISKEL_INDEXED_FRAME_WIDTH * PLAYER_SPRITE_SCALE;

    /* Centre the sprite around moving_x */
    sprite_x = moving_x - (sprite_width_scaled / 2);

    LCD_Draw_Sprite_Scaled(
        (uint16_t)sprite_x,
        (uint16_t)PLAYER_SPRITE_Y,
        NEW_PISKEL_INDEXED_FRAME_WIDTH,
        NEW_PISKEL_INDEXED_FRAME_HEIGHT,
        (const uint8_t *)new_piskel_data_indexed,
        PLAYER_SPRITE_SCALE
    );
}

void Game1_Init(void)
{
    int i;

    animation_counter = 0;
    moving_x = 100;
    move_direction = 1;
    exit_requested = 0;
    player_lane = 1;
    scroll_y = 0;
    next_spawn_lane = 0;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        obstacle_active[i] = 0;
        obstacle_y[i] = 0;
        obstacle_lane[i] = 0;
    }
}

void Game1_Reset(void)
{
    int i;

    animation_counter = 0;
    moving_x = 100;
    move_direction = 1;
    exit_requested = 0;
    player_lane = 1;
    scroll_y = 0;
    next_spawn_lane = 0;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        obstacle_active[i] = 0;
        obstacle_y[i] = 0;
        obstacle_lane[i] = 0;
    }
}

void Game1_HandleInput(void)
{
    Input_Read();

    if (current_input.btn3_pressed) {
        exit_requested = 1;
    }
}

void Game1_UpdatePlayerLane(void)
{
    if ((animation_counter % 40) == 0) {
        if (player_lane >= 2) {
            move_direction = -1;
        } else if (player_lane <= 0) {
            move_direction = 1;
        }

        player_lane += move_direction;
    }

    if (moving_x < lane_x[player_lane]) {
        moving_x += 5;
        if (moving_x > lane_x[player_lane]) {
            moving_x = lane_x[player_lane];
        }
    } else if (moving_x > lane_x[player_lane]) {
        moving_x -= 5;
        if (moving_x < lane_x[player_lane]) {
            moving_x = lane_x[player_lane];
        }
    }
}

void Game1_UpdateWorldScroll(void)
{
    scroll_y += 4;

    if (scroll_y >= 40) {
        scroll_y = 0;
    }
}

void Game1_SpawnSegment(void)
{
    int i;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i] == 0) {
            obstacle_y[i] = 20;
            obstacle_lane[i] = next_spawn_lane;
            obstacle_active[i] = 1;

            next_spawn_lane++;
            if (next_spawn_lane > 2) {
                next_spawn_lane = 0;
            }

            break;
        }
    }
}

MenuState Game1_Run(void)
{
    int i;

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
        Game1_UpdateWorldScroll();

        if ((animation_counter % 50) == 0) {
            Game1_SpawnSegment();
        }

        {
            uint8_t brightness = (moving_x * 100) / 200;
            PWM_SetDuty(&pwm_cfg, brightness);
        }

        LCD_Fill_Buffer(0);

        LCD_printString("GAME 1", 60, 10, 1, 3);

        /* Road markers */
        LCD_printString("|", 40, 110 + scroll_y, 1, 2);
        LCD_printString("|", 100, 110 + scroll_y, 1, 2);
        LCD_printString("|", 160, 110 + scroll_y, 1, 2);

        LCD_printString("|", 40, 150 + scroll_y, 1, 2);
        LCD_printString("|", 100, 150 + scroll_y, 1, 2);
        LCD_printString("|", 160, 150 + scroll_y, 1, 2);

        LCD_printString("|", 40, 190 + scroll_y, 1, 2);
        LCD_printString("|", 100, 190 + scroll_y, 1, 2);
        LCD_printString("|", 160, 190 + scroll_y, 1, 2);

        /* Obstacles */
        for (i = 0; i < OBSTACLE_COUNT; i++) {
            if (obstacle_active[i]) {
                LCD_printString("[#]", lane_x[obstacle_lane[i]], obstacle_y[i] + scroll_y, 1, 2);
            }
        }

        /* Player sprite at 2x scale */
        Game1_DrawPlayerSprite();

        {
            char counter[32];
            sprintf(counter, "Frame: %lu", animation_counter);
            LCD_printString(counter, 10, 80, 1, 1);
        }

        {
            char lane_str[32];
            sprintf(lane_str, "Lane: %d", player_lane);
            LCD_printString(lane_str, 10, 92, 1, 1);
        }

        {
            char scroll_str[32];
            sprintf(scroll_str, "Scroll: %d", scroll_y);
            LCD_printString(scroll_str, 10, 104, 1, 1);
        }

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