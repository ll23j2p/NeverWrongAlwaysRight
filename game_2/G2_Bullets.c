#include "Game_2.h"
#include "LCD.h"
#include <stdint.h>

static G2_Bullet pbullets[G2_BULLET_MAX];
static G2_Bullet ebullets[G2_BULLET_MAX];

void G2_Bullets_Init(void){
    for(int i= 0; i < G2_BULLET_MAX; i++){
        pbullets[i].active = 0;
    }
    for(int i = 0; i < G2_BULLET_MAX; i++){
        ebullets[i].active = 0;
    }
}

/* ===== Player Bullets ===== */
uint8_t G2_Bullets_PlayerShoot(int16_t x, int16_t y){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!pbullets[i].active){
            pbullets[i].x = x;
            pbullets[i].y = y;
            pbullets[i].active = 1;
            return 1;
        }
    }
    return 0;
}

void G2_Bullets_UpdatePlayer(void){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!pbullets[i].active){
            continue;
        }
        pbullets[i].y -= G2_BULLET_PSPEED;
        if(pbullets[i].y < (int16_t)G2_TOP){
            pbullets[i].active = 0;
        }
    }
}

void G2_Bullets_DrawPlayer(void){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!pbullets[i].active){
            continue;
        }
        LCD_Draw_Sprite_Colour((uint16_t)pbullets[i].x, (uint16_t)pbullets[i].y, G2_BULLET_H, G2_BULLET_W, G2_SPR_BULLET, G2_YELLOW);
    }
}

G2_Bullet *G2_Bullets_GetPlayerPool(void){
    return pbullets;
}

/* ===== Enemy Bullets ===== */
uint8_t G2_Bullets_EnemyShoot(int16_t x, int16_t y){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!ebullets[i].active){
            ebullets[i].x = x;
            ebullets[i].y = y;
            ebullets[i].active = 1;
            return 1;
        }
    }
    return 0;
}

void G2_Bullets_UpdateEnemy(void){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!ebullets[i].active){
            continue;
        }
        ebullets[i].y += G2_BULLET_ESPEED;
        if(ebullets[i].y > (int16_t)G2_LCD_H){
            ebullets[i].active = 0;
        }
    }
}

void G2_Bullets_DrawEnemy(void){
    for(int i = 0; i < G2_BULLET_MAX; i++){
        if(!ebullets[i].active){
            continue;
        }
        LCD_Draw_Sprite_Colour((uint16_t)ebullets[i].x, (uint16_t)ebullets[i].y, G2_BULLET_H, G2_BULLET_W, G2_SPR_BULLET, G2_BLUE);
    }
}

G2_Bullet *G2_Bullets_GetEnemyPool(void){
    return ebullets;
}