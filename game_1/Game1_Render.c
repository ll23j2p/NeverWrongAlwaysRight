#include "Game1_Render.h"
#include "Game1_Logic.h"
#include "Game1_Config.h"
#include "LCD.h"
#include "assets/Player_Piskel_indexed.h"
#include "assets/New_Piskel_indexed.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;

/* ---------- small internal helpers private to Game1_Render.c ---------- */

// converts a raw Piskel AABBGGRR pixel value to an LCD palette index
// returns LCD_TRANSPARENT_INDEX (255) for fully transparent pixels
static uint8_t Game1_ConvertPiskelPixelToPaletteIndex(uint32_t pixel)
{
    if      (pixel == 0x00000000u) return LCD_TRANSPARENT_INDEX;
    else if (pixel == 0xff000000u) return LCD_BLACK_INDEX;
    else if (pixel == 0xffffffffu) return LCD_WHITE_INDEX;
    else if (pixel == 0xff0000ffu) return LCD_RED_INDEX;
    else if (pixel == 0xff00ff00u) return LCD_GREEN_INDEX;
    else if (pixel == 0xffff0000u) return LCD_BLUE_INDEX;
    else if (pixel == 0xff00ffffu) return LCD_YELLOW_INDEX;
    else if (pixel == 0xffffff00u) return LCD_CYAN_INDEX;
    else if (pixel == 0xffff00ffu) return LCD_MAGENTA_INDEX;
    else if (pixel == 0xff000088u) return LCD_DARK_RED_INDEX;
    else if (pixel == 0xff008800u) return LCD_DARK_GREEN_INDEX;
    else if (pixel == 0xff880000u) return LCD_DARK_BLUE_INDEX;
    else if (pixel == 0xff0088ffu) return LCD_ORANGE_INDEX;
    else if (pixel == 0xff880088u) return LCD_PURPLE_INDEX;
    else if (pixel == 0xff888800u) return LCD_TEAL_INDEX;
    else if (pixel == 0xff004488u) return LCD_BROWN_INDEX;
    else if (pixel == 0xff888888u) return LCD_GREY_INDEX;

    return LCD_GREY_INDEX; // unknown colour shown as grey
}

// draws the player sprite at (x, y) -- each source pixel becomes a PLAYER_SPRITE_SCALE rect
// transparent pixels are skipped so the background shows through
static void Game1_DrawPlayerSpriteAt(int16_t x, int16_t y)
{
    uint16_t row, col, index;
    uint32_t raw_pixel;
    uint8_t  palette_pixel;

    for (row = 0; row < NEW_PISKEL_FRAME_HEIGHT; row++) {
        for (col = 0; col < NEW_PISKEL_FRAME_WIDTH; col++) {
            index         = (row * NEW_PISKEL_FRAME_WIDTH) + col;
            raw_pixel     = new_piskel_data[0][index];
            palette_pixel = Game1_ConvertPiskelPixelToPaletteIndex(raw_pixel);

            if (palette_pixel != LCD_TRANSPARENT_INDEX) {
                LCD_Draw_Rect(
                    x + (col * PLAYER_SPRITE_SCALE),
                    y + (row * PLAYER_SPRITE_SCALE),
                    PLAYER_SPRITE_SCALE,
                    PLAYER_SPRITE_SCALE,
                    palette_pixel, 1
                );
            }
        }
    }
}

/* ---------- required drawing functions ---------- */

// draws the obstacle sprite scaled to target_size and centred on (centre_x, centre_y)
// uses nearest-neighbour scaling: output pixel maps back to source via integer division
void Game1_DrawObstacleSpriteAt(int16_t centre_x, int16_t centre_y, int16_t target_size)
{
    int16_t row, col, source_row, source_col;
    uint8_t pixel;

    if (target_size <= 0) return; // nothing to draw

    int16_t draw_x = centre_x - (target_size / 2);
    int16_t draw_y = centre_y - (target_size / 2);

    for (row = 0; row < target_size; row++) {
        source_row = (row * NEW_PISKEL_INDEXED_FRAME_HEIGHT) / target_size;

        for (col = 0; col < target_size; col++) {
            source_col = (col * NEW_PISKEL_INDEXED_FRAME_WIDTH) / target_size;
            pixel      = new_piskel_data_indexed[source_row][source_col];

            if (pixel != NEW_PISKEL_INDEXED_TRANSPARENT) {
                LCD_Draw_Rect(draw_x + col, draw_y + row, 1, 1, pixel, 1);
            }
        }
    }
}

// draws four converging lane lines (static) hence then overlays scrolling dashed markers
// All markers advance using animation_counter * obstacle_speed and wrap every MARKER_SPACING
// Evary dash is projected through Game1_ProjectToScreen so it scales with depth automatically
void Game1_DrawTrack(void)
{
    // static lane boundary lines -- narrow at horizon, wide at near edge
    LCD_Draw_Line(SCREEN_CENTRE_X - (FAR_LANE_SPACING  * 2), HORIZON_Y,
                  SCREEN_CENTRE_X - (NEAR_LANE_SPACING * 2), NEAR_Y, 1);
    LCD_Draw_Line(SCREEN_CENTRE_X + (FAR_LANE_SPACING  * 2), HORIZON_Y,
                  SCREEN_CENTRE_X + (NEAR_LANE_SPACING * 2), NEAR_Y, 1);
    LCD_Draw_Line(SCREEN_CENTRE_X - FAR_LANE_SPACING,  HORIZON_Y,
                  SCREEN_CENTRE_X - NEAR_LANE_SPACING, NEAR_Y, 1);
    LCD_Draw_Line(SCREEN_CENTRE_X + FAR_LANE_SPACING,  HORIZON_Y,
                  SCREEN_CENTRE_X + NEAR_LANE_SPACING, NEAR_Y, 1);

    // scrolling dashed centre markers
    {
        int8_t  marker_lanes[2] = {0, 2}; // left and right lane centres
        int16_t scroll_offset   = (int16_t)((animation_counter * obstacle_speed) % MARKER_SPACING);
        int16_t marker_world_y, screen_x, screen_y, dash_size;
        int m;

        for (m = 0; m < 2; m++) {
            for (marker_world_y = scroll_offset;
                 marker_world_y < WORLD_MAX_Y;
                 marker_world_y += MARKER_SPACING)
            {
                if (marker_world_y < 10) continue; // too small to see near horizon

                Game1_ProjectToScreen(
                    marker_lanes[m], marker_world_y,
                    &screen_x, &screen_y, &dash_size
                );

                if (screen_y >= HORIZON_Y && screen_y <= NEAR_Y) {
                    // dash height scales with depth so markers look perspective-correct
                    LCD_Draw_Rect(screen_x - 1, screen_y, 2,
                                  (dash_size / 4) + 1, LCD_WHITE_INDEX, 1);
                }
            }
        }
    }
}

// To draw all active obstacles both projected and scaled ones then the player sprite
void Game1_DrawEntities(void)
{
    int i;
    int16_t screen_x, screen_y, size;

    for (i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacle_active[i]) {
            Game1_ProjectToScreen(
                obstacle_lane[i], obstacle_world_y[i],
                &screen_x, &screen_y, &size
            );
            Game1_DrawObstacleSpriteAt(screen_x, screen_y, size);
        }
    }

    // draw player centred on moving_x at the fixed near-screen Y position
    int16_t player_x = moving_x - ((NEW_PISKEL_FRAME_WIDTH * PLAYER_SPRITE_SCALE) / 2);
    Game1_DrawPlayerSpriteAt(player_x, PLAYER_SCREEN_Y);
}

// Shows score, lives, sped and lane number; also GAME OVER prompt when game finished 
void Game1_DrawHUD(void)
{
    char buf[32];

    sprintf(buf, "Lane:%d",   player_lane);    LCD_printString(buf, 10, 12, 1, 1);
    sprintf(buf, "Score:%u",  score);          LCD_printString(buf, 10, 24, 1, 1);
    sprintf(buf, "Speed:%d",  obstacle_speed); LCD_printString(buf, 10, 36, 1, 1);
    sprintf(buf, "Lives:%d",  lives);          LCD_printString(buf, 10, 48, 1, 1);

    if (game_over) {
        LCD_printString("GAME OVER",   55, 95,  1, 3);
        LCD_printString("BT3 restart", 60, 130, 1, 2);
    } else {
        LCD_printString("Joystick lane", 58, 216, 1, 1);
        LCD_printString("BT3 menu",      75, 228, 1, 1);
    }
}

// clears the buffer then composes the full frame in order: title -> track -> entities -> HUD
// called every frame regardless of game state
void Game1_RenderFrame(void)
{
    LCD_Fill_Buffer(0);
    LCD_printString("GAME 1", 60, 10, 1, 3);

    Game1_DrawTrack();
    Game1_DrawEntities();
    Game1_DrawHUD();

    LCD_Refresh(&cfg0);
}
