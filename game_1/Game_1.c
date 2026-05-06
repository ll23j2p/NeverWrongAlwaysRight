#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "adc.h"
#include "stm32l4xx_hal.h"
#include "assets/Player_Piskel_indexed.h"
#include "assets/New_Piskel_indexed.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;
extern ADC_HandleTypeDef hadc1;

/* ===== GAME STATE ===== */
static uint32_t animation_counter = 0;
static int16_t moving_x = 120;
static uint8_t exit_requested = 0;
static int8_t player_lane = 1;
static uint8_t lane_move_ready = 1;

static uint16_t score = 0;
static uint8_t lives = 3;
static uint8_t game_over = 0;
static int16_t obstacle_speed = 4;

/* World positions */
#define OBSTACLE_COUNT 4
static uint8_t obstacle_active[OBSTACLE_COUNT] = {0, 0, 0, 0};
static int16_t obstacle_world_y[OBSTACLE_COUNT] = {0, 0, 0, 0};
static int8_t obstacle_lane[OBSTACLE_COUNT] = {0, 0, 0, 0};
static int8_t next_spawn_lane = 0;

/* ===== JOYSTICK ===== */
static Joystick_cfg_t joystick_cfg = {
    .adc = &hadc1,
    .x_channel = ADC_CHANNEL_1,
    .y_channel = ADC_CHANNEL_2,
    .sampling_time = ADC_SAMPLETIME_47CYCLES_5,
    .center_x = JOYSTICK_DEFAULT_CENTER_X,
    .center_y = JOYSTICK_DEFAULT_CENTER_Y,
    .deadzone = JOYSTICK_DEADZONE,
    .setup_done = 0
};

static Joystick_t joystick_data;
static UserInput joystick_input;

/* ===== GAME CONFIG ===== */
#define GAME1_FRAME_TIME_MS 30
#define PLAYER_SPRITE_SCALE 2

#define SCREEN_CENTRE_X 120
#define HORIZON_Y 70
#define NEAR_Y 210
#define WORLD_MAX_Y 200

#define FAR_LANE_SPACING 18
#define NEAR_LANE_SPACING 60

#define PLAYER_SCREEN_Y 165
#define PLAYER_COLLISION_Y_MIN 150
#define PLAYER_COLLISION_Y_MAX 205

#define OBSTACLE_MIN_SIZE 8
#define OBSTACLE_MAX_SIZE 24

/* LCD palette indices */
#define LCD_TRANSPARENT_INDEX 255
#define LCD_BLACK_INDEX       0
#define LCD_WHITE_INDEX       1
#define LCD_RED_INDEX         2
#define LCD_GREEN_INDEX       3
#define LCD_BLUE_INDEX        4
#define LCD_YELLOW_INDEX      5
#define LCD_CYAN_INDEX        6
#define LCD_MAGENTA_INDEX     7
#define LCD_DARK_RED_INDEX    8
#define LCD_DARK_GREEN_INDEX  9
#define LCD_DARK_BLUE_INDEX   10
#define LCD_ORANGE_INDEX      11
#define LCD_PURPLE_INDEX      12
#define LCD_TEAL_INDEX        13
#define LCD_BROWN_INDEX       14
#define LCD_GREY_INDEX        15

static uint8_t Game1_ConvertPiskelPixelToPaletteIndex(uint32_t pixel)
{
    /*
     * Piskel raw export uses 0xAABBGGRR format.
     * LCD drawing uses palette index values 0 to 15.
     */

    if (pixel == 0x00000000u) {
        return LCD_TRANSPARENT_INDEX;
    }
    else if (pixel == 0xff000000u) {
        return LCD_BLACK_INDEX;
    }
    else if (pixel == 0xffffffffu) {
        return LCD_WHITE_INDEX;
    }
    else if (pixel == 0xff0000ffu) {
        return LCD_RED_INDEX;
    }
    else if (pixel == 0xff00ff00u) {
        return LCD_GREEN_INDEX;
    }
    else if (pixel == 0xffff0000u) {
        return LCD_BLUE_INDEX;
    }
    else if (pixel == 0xff00ffffu) {
        return LCD_YELLOW_INDEX;
    }
    else if (pixel == 0xffffff00u) {
        return LCD_CYAN_INDEX;
    }
    else if (pixel == 0xffff00ffu) {
        return LCD_MAGENTA_INDEX;
    }
    else if (pixel == 0xff000088u) {
        return LCD_DARK_RED_INDEX;
    }
    else if (pixel == 0xff008800u) {
        return LCD_DARK_GREEN_INDEX;
    }
    else if (pixel == 0xff880000u) {
        return LCD_DARK_BLUE_INDEX;
    }
    else if (pixel == 0xff0088ffu) {
        return LCD_ORANGE_INDEX;
    }
    else if (pixel == 0xff880088u) {
        return LCD_PURPLE_INDEX;
    }
    else if (pixel == 0xff888800u) {
        return LCD_TEAL_INDEX;
    }
    else if (pixel == 0xff004488u) {
        return LCD_BROWN_INDEX;
    }
    else if (pixel == 0xff888888u) {
        return LCD_GREY_INDEX;
    }

    /*
     * TEST MODE:
     * Unknown colours are shown as grey instead of hidden.
     * Once the player sprite works correctly, change this back to:
     * return LCD_TRANSPARENT_INDEX;
     */
    return LCD_GREY_INDEX;
}

static void Game1_DrawPlayerSpriteAt(int16_t x, int16_t y)
{
    uint16_t row;
    uint16_t col;
    uint16_t index;
    uint32_t raw_pixel;
    uint8_t palette_pixel;

    for (row = 0; row < NEW_PISKEL_FRAME_HEIGHT; row++) {
        for (col = 0; col < NEW_PISKEL_FRAME_WIDTH; col++) {
            index = (row * NEW_PISKEL_FRAME_WIDTH) + col;
            raw_pixel = new_piskel_data[0][index];

            palette_pixel = Game1_ConvertPiskelPixelToPaletteIndex(raw_pixel);

            if (palette_pixel != LCD_TRANSPARENT_INDEX) {
                LCD_Draw_Rect(
                    x + (col * PLAYER_SPRITE_SCALE),
                    y + (row * PLAYER_SPRITE_SCALE),
                    PLAYER_SPRITE_SCALE,
                    PLAYER_SPRITE_SCALE,
                    palette_pixel,
                    1
                );
            }
        }
    }
}

void Game1_DrawObstacleSpriteAt(int16_t centre_x, int16_t centre_y, int16_t target_size)
{
    int16_t row;
    int16_t col;
    int16_t source_row;
    int16_t source_col;
    int16_t draw_x;
    int16_t draw_y;
    uint8_t pixel;

    if (target_size <= 0) {
        return;
    }

    draw_x = centre_x - (target_size / 2);
    draw_y = centre_y - (target_size / 2);

    for (row = 0; row < target_size; row++) {
        source_row = (row * NEW_PISKEL_INDEXED_FRAME_HEIGHT) / target_size;

        for (col = 0; col < target_size; col++) {
            source_col = (col * NEW_PISKEL_INDEXED_FRAME_WIDTH) / target_size;

            pixel = new_piskel_data_indexed[source_row][source_col];

            if (pixel != NEW_PISKEL_INDEXED_TRANSPARENT) {
                LCD_Draw_Rect(
                    draw_x + col,
                    draw_y + row,
                    1,
                    1,
                    pixel,
                    1
                );
            }
        }
    }
}

void Game1_Init(void)
{
    int i;

    animation_counter = 0;
    moving_x = SCREEN_CENTRE_X;
    exit_requested = 0;
    player_lane = 1;
    lane_move_ready = 1;

    score = 0;
    lives = 3;
    game_over = 0;
    obstacle_speed = 4;
    next_spawn_lane = 0;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        obstacle_active[i] = 0;
        obstacle_world_y[i] = 0;
        obstacle_lane[i] = 0;
    }

    Joystick_Init(&joystick_cfg);
    Joystick_Calibrate(&joystick_cfg);
}

void Game1_Reset(void)
{
    Game1_Init();
}

void Game1_HandleInput(void)
{
    Input_Read();

    Joystick_Read(&joystick_cfg, &joystick_data);
    joystick_input = Joystick_GetInput(&joystick_data);

    if (game_over) {
        if (current_input.btn3_pressed) {
            Game1_Reset();
        }
        return;
    }

    if (current_input.btn3_pressed) {
        exit_requested = 1;
    }
}

void Game1_UpdatePlayerLane(void)
{
    int16_t target_x;

    if (joystick_input.direction == CENTRE) {
        lane_move_ready = 1;
    }

    if (lane_move_ready) {
        if (joystick_input.direction == W ||
            joystick_input.direction == NW ||
            joystick_input.direction == SW) {

            if (player_lane > 0) {
                player_lane--;
            }

            lane_move_ready = 0;
        }
        else if (joystick_input.direction == E ||
                 joystick_input.direction == NE ||
                 joystick_input.direction == SE) {

            if (player_lane < 2) {
                player_lane++;
            }

            lane_move_ready = 0;
        }
    }

    if (player_lane == 0) {
        target_x = SCREEN_CENTRE_X - NEAR_LANE_SPACING;
    }
    else if (player_lane == 1) {
        target_x = SCREEN_CENTRE_X;
    }
    else {
        target_x = SCREEN_CENTRE_X + NEAR_LANE_SPACING;
    }

    if (moving_x < target_x) {
        moving_x += 6;

        if (moving_x > target_x) {
            moving_x = target_x;
        }
    }
    else if (moving_x > target_x) {
        moving_x -= 6;

        if (moving_x < target_x) {
            moving_x = target_x;
        }
    }
}

void Game1_UpdateWorldScroll(void)
{
    animation_counter++;

    if ((animation_counter % 200) == 0) {
        if (obstacle_speed < 8) {
            obstacle_speed++;
        }
    }
}

void Game1_SpawnSegment(void)
{
    int i;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i] == 0) {
            obstacle_active[i] = 1;
            obstacle_world_y[i] = 0;
            obstacle_lane[i] = next_spawn_lane;

            next_spawn_lane++;

            if (next_spawn_lane > 2) {
                next_spawn_lane = 0;
            }

            break;
        }
    }
}

void Game1_UpdateSegments(void)
{
    int i;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i]) {
            obstacle_world_y[i] += obstacle_speed;

            if (obstacle_world_y[i] > WORLD_MAX_Y) {
                obstacle_active[i] = 0;
                obstacle_world_y[i] = 0;
                score += 10;
            }
        }
    }
}

void Game1_CheckCollisions(void)
{
    int i;
    int16_t screen_x;
    int16_t screen_y;
    int16_t size;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i]) {
            if (obstacle_lane[i] == player_lane) {
                Game1_ProjectToScreen(
                    obstacle_lane[i],
                    obstacle_world_y[i],
                    &screen_x,
                    &screen_y,
                    &size
                );

                if (screen_y >= PLAYER_COLLISION_Y_MIN &&
                    screen_y <= PLAYER_COLLISION_Y_MAX) {

                    obstacle_active[i] = 0;
                    obstacle_world_y[i] = 0;

                    if (lives > 0) {
                        lives--;
                    }

                    buzzer_tone(&buzzer_cfg, 1500, 40);
                    HAL_Delay(30);
                    buzzer_off(&buzzer_cfg);

                    if (lives == 0) {
                        game_over = 1;
                    }
                }
            }
        }
    }
}

void Game1_UpdateScoreSpeed(void)
{
    if (!game_over) {
        score++;
    }
}

void Game1_ProjectToScreen(
    int8_t lane,
    int16_t world_y,
    int16_t *screen_x,
    int16_t *screen_y,
    int16_t *size
)
{
    int16_t lane_spacing;
    int16_t lane_offset;

    *screen_y = HORIZON_Y + ((world_y * (NEAR_Y - HORIZON_Y)) / WORLD_MAX_Y);

    lane_spacing = FAR_LANE_SPACING +
                   ((world_y * (NEAR_LANE_SPACING - FAR_LANE_SPACING)) / WORLD_MAX_Y);

    lane_offset = lane - 1;
    *screen_x = SCREEN_CENTRE_X + (lane_offset * lane_spacing);

    *size = OBSTACLE_MIN_SIZE +
            ((world_y * (OBSTACLE_MAX_SIZE - OBSTACLE_MIN_SIZE)) / WORLD_MAX_Y);
}

void Game1_DrawTrack(void)
{
    int16_t left_top_x;
    int16_t right_top_x;
    int16_t left_bottom_x;
    int16_t right_bottom_x;

    left_top_x = SCREEN_CENTRE_X - (FAR_LANE_SPACING * 2);
    right_top_x = SCREEN_CENTRE_X + (FAR_LANE_SPACING * 2);

    left_bottom_x = SCREEN_CENTRE_X - (NEAR_LANE_SPACING * 2);
    right_bottom_x = SCREEN_CENTRE_X + (NEAR_LANE_SPACING * 2);

    LCD_Draw_Line(left_top_x, HORIZON_Y, left_bottom_x, NEAR_Y, 1);
    LCD_Draw_Line(right_top_x, HORIZON_Y, right_bottom_x, NEAR_Y, 1);

    LCD_Draw_Line(
        SCREEN_CENTRE_X - FAR_LANE_SPACING,
        HORIZON_Y,
        SCREEN_CENTRE_X - NEAR_LANE_SPACING,
        NEAR_Y,
        1
    );

    LCD_Draw_Line(
        SCREEN_CENTRE_X + FAR_LANE_SPACING,
        HORIZON_Y,
        SCREEN_CENTRE_X + NEAR_LANE_SPACING,
        NEAR_Y,
        1
    );
}

void Game1_DrawEntities(void)
{
    int i;
    int16_t screen_x;
    int16_t screen_y;
    int16_t size;
    int16_t player_sprite_x;
    int16_t player_sprite_width_scaled;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i]) {
            Game1_ProjectToScreen(
                obstacle_lane[i],
                obstacle_world_y[i],
                &screen_x,
                &screen_y,
                &size
            );

            Game1_DrawObstacleSpriteAt(screen_x, screen_y, size);
        }
    }

    player_sprite_width_scaled = NEW_PISKEL_FRAME_WIDTH * PLAYER_SPRITE_SCALE;
    player_sprite_x = moving_x - (player_sprite_width_scaled / 2);

    Game1_DrawPlayerSpriteAt(player_sprite_x, PLAYER_SCREEN_Y);
}

void Game1_DrawHUD(void)
{
    char lane_str[32];
    char score_str[32];
    char speed_str[32];
    char lives_str[32];

    sprintf(lane_str, "Lane:%d", player_lane);
    LCD_printString(lane_str, 10, 12, 1, 1);

    sprintf(score_str, "Score:%u", score);
    LCD_printString(score_str, 10, 24, 1, 1);

    sprintf(speed_str, "Speed:%d", obstacle_speed);
    LCD_printString(speed_str, 10, 36, 1, 1);

    sprintf(lives_str, "Lives:%d", lives);
    LCD_printString(lives_str, 10, 48, 1, 1);

    if (game_over) {
        LCD_printString("GAME OVER", 55, 95, 1, 3);
        LCD_printString("BT3 restart", 60, 130, 1, 2);
    }
    else {
        LCD_printString("Joystick lane", 58, 216, 1, 1);
        LCD_printString("BT3 menu", 75, 228, 1, 1);
    }
}

void Game1_RenderFrame(void)
{
    LCD_Fill_Buffer(0);

    LCD_printString("GAME 1", 60, 10, 1, 3);

    Game1_DrawTrack();
    Game1_DrawEntities();
    Game1_DrawHUD();

    LCD_Refresh(&cfg0);
}

MenuState Game1_Run(void)
{
    uint32_t frame_start;
    uint32_t frame_time;

    Game1_Init();

    buzzer_tone(&buzzer_cfg, 1000, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);

    while (1) {
        frame_start = HAL_GetTick();

        Game1_HandleInput();

        if (exit_requested) {
            PWM_SetDuty(&pwm_cfg, 50);
            return MENU_STATE_HOME;
        }

        if (!game_over) {
            Game1_UpdatePlayerLane();
            Game1_UpdateWorldScroll();

            if ((animation_counter % 35) == 0) {
                Game1_SpawnSegment();
            }

            Game1_UpdateSegments();
            Game1_CheckCollisions();
            Game1_UpdateScoreSpeed();

            {
                uint8_t brightness = (moving_x * 100) / 240;
                PWM_SetDuty(&pwm_cfg, brightness);
            }
        }

        Game1_RenderFrame();

        frame_time = HAL_GetTick() - frame_start;

        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }
}