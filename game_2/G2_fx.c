#include "Game_2.h"
#include "LCD.h"
#include "Utils.h"
#include <stdint.h>

/* ===== Space ===== */
typedef struct{
    int16_t x;
    int16_t y;
    uint8_t speed;
    uint8_t colour;
}Spacefield;

/* ===== Speedlines ===== */
typedef struct{
    int16_t x;
    int16_t y;
    uint8_t length;
    uint8_t speed;
}Speedlines;

typedef struct{
    int16_t x;
    int16_t y;
    uint8_t frame;
    uint8_t timer;
    uint8_t active;
}Explosion;

static Spacefield spacefields[G2_SPACEFIELD_COUNT];
static Speedlines speedlines[G2_SPEEDLINE_COUNT];
static Explosion explosions[G2_EXPLOSION_MAX];

/* ===== Spacefield ===== */
void G2_FX_SpacefieldInit(void){
    for(int i = 0; i < G2_SPACEFIELD_COUNT; i++){
        spacefields[i].x = (int16_t)Random_U16(G2_LCD_W);
        spacefields[i].y = (int16_t)Random_U16(G2_LCD_H);
        spacefields[i].speed = (uint8_t)(1 + Random_U16(3));
 
        uint16_t band = Random_U16(3);
        if(band == 0){
            spacefields[i].colour = G2_WHITE;
        }else if(band == 1){
            spacefields[i].colour = G2_GREY;
        }else{
            spacefields[i].colour = G2_CYAN;
        }
    }
}
 
void G2_FX_SpacefieldUpdateDraw(void){
    for(int i = 0; i < G2_SPACEFIELD_COUNT; ++i) {
        spacefields[i].y += (int16_t)spacefields[i].speed;
 
        if (spacefields[i].y >= (int16_t)G2_LCD_H){
            spacefields[i].y = 0;
            spacefields[i].x = (int16_t)Random_U16(G2_LCD_W);
        }
 
        LCD_Draw_Rect((uint16_t)spacefields[i].x, (uint16_t)spacefields[i].y,
                        1, 1, spacefields[i].colour, 1); //line too long
    }
}

/* ===== Speedlines ===== */
void G2_FX_SpeedLinesInit(void){
    for(int i = 0; i < G2_SPEEDLINE_COUNT; i++){
        speedlines[i].x      = (int16_t)Random_U16(G2_LCD_W);
        speedlines[i].y      = (int16_t)Random_U16(G2_LCD_H);
        speedlines[i].length = (uint8_t)(8 + Random_U16(8));
        speedlines[i].speed  = (uint8_t)(8 + Random_U16(8));
    }
}
 
void G2_FX_SpeedLinesUpdateDraw(void){
    for(uint8_t i = 0; i < G2_SPEEDLINE_COUNT; i++){
        speedlines[i].y += (int16_t)speedlines[i].speed;
 
        if(speedlines[i].y > (int16_t)G2_LCD_H){
            speedlines[i].y = (int16_t)(-(int16_t)speedlines[i].length);
            speedlines[i].x = (int16_t)Random_U16(G2_LCD_W);
            speedlines[i].length = (uint8_t)(8 + Random_U16(8));
            speedlines[i].speed  = (uint8_t)(8 + Random_U16(8));
        }

        if(speedlines[i].y < 0){
            int16_t visible = (int16_t)((int16_t)speedlines[i].length + speedlines[i].y);
            if(visible > 0){
                LCD_Draw_Rect((uint16_t)speedlines[i].x, 0, 1, (uint16_t)visible, G2_WHITE, 1);
            }
        }else{
            LCD_Draw_Rect((uint16_t)speedlines[i].x, (uint16_t)speedlines[i].y, 1, 
            (uint16_t)speedlines[i].length, G2_WHITE, 1);
        }
    }
}

/* ===== Explosions ===== */
void G2_FX_ExplosionsInit(void){
    for(int i = 0; i < G2_EXPLOSION_MAX; i++)
        explosions[i].active = 0;
}
 
void G2_FX_Explosion(int16_t x, int16_t y){
    for (int i = 0; i < G2_EXPLOSION_MAX; i++) {
        if (!explosions[i].active) {
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].frame = 0;
            explosions[i].timer = 0;
            explosions[i].active = 1;
            return;
        }
    }
}
 
void G2_FX_ExplosionsUpdateDraw(void){
    for(int i = 0; i < G2_EXPLOSION_MAX; i++){
        if (!explosions[i].active){
            continue;
        }
        explosions[i].timer++;
        if(explosions[i].timer >= G2_EXPLOSION_TIMING){
            explosions[i].timer = 0;
            explosions[i].frame++;
        }
 
        if(explosions[i].frame >= G2_EXPLOSION_FRAME){
            explosions[i].active = 0;
            continue;
        }
 
        const uint8_t *sprite;
        if(explosions[i].frame & 1){
            sprite = G2_SPR_EXPLODE1;
        }else{
            sprite = G2_SPR_EXPLODE2;
        }
 
        uint8_t colour;
        if(explosions[i].frame < 2){
            colour = G2_YELLOW;
        }else if(explosions[i].frame < 4){
            colour = G2_ORANGE;
        }else{
            colour = G2_RED;
        }
 
        LCD_Draw_Sprite_Colour((uint16_t)explosions[i].x, (uint16_t)explosions[i].y, G2_EXPLODE_H, G2_EXPLODE_W, sprite, colour);
    }
}