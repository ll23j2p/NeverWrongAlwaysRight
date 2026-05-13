#include "Game_2.h"
#include "LCD.h"
#include "Utils.h"
#include <ctype.h>
#include <stdint.h>

/* ===== Structs and states ===== */
typedef struct{
    int16_t x;
    int16_t y;
    uint8_t active;
    uint8_t type;
    uint8_t hp;
    uint8_t cd;
    int8_t dx;
}Enemy;

static Enemy  enemies[G2_MAX_ENEMIES];
static uint16_t wave;
static uint8_t  to_spawn;
static uint8_t  spawn_cd;

static void Enemy_kill(uint8_t i);

void G2_Enemy_Init(void){
    for(int i = 0; i < G2_MAX_ENEMIES; i++){
        enemies[i].active = 0;
    }
    wave = 1;
    to_spawn = G2_WAVE_BASE_COUNT + (uint8_t)(wave*G2_WAVE_PER_WAVE);
    spawn_cd = 30;
}

/* ===== Wave management ===== */
void G2_Enemy_NextWave(void){
    wave++;
    uint16_t target = (uint16_t)(G2_WAVE_BASE_COUNT+wave*G2_WAVE_PER_WAVE);
    if(target>G2_WAVE_MAX_COUNT){
        target = G2_WAVE_MAX_COUNT;
    }
    to_spawn = (uint8_t)target;
    spawn_cd = 30;
}

uint16_t G2_Enemy_GetWave(void){
    return wave;
}

uint8_t G2_Enemy_WaveCleared(void){
    if(to_spawn > 0){
        return 0;
    }
    for(int i = 0; i<G2_MAX_ENEMIES; i++){
        if(enemies[i].active){
            return 0;
        }
    }
    return 1;
}

/* ===== Spawn Enemies ===== */
static void Enemy_spawn(void){
    for(int i = 0; i < G2_MAX_ENEMIES; i++){
        if (enemies[i].active){
            continue;
        }
        enemies[i].active = 1;
        enemies[i].x = (int16_t)Random_U16(G2_LCD_W - G2_ENEMY_W);
        enemies[i].y = (int16_t)(G2_TOP - G2_ENEMY_H);
        enemies[i].type = (uint8_t)(Random_U16(2));
 
        if(Random_U16(2)){
            enemies[i].dx = 1;
        }else{
            enemies[i].dx = -1;
        }
 
        enemies[i].cd = (uint8_t)(30 + Random_U16(60));
 
        uint16_t hp = 1 + (wave/3);
        if(hp > 4){
            hp = 4;
        }
        enemies[i].hp = (uint8_t)hp;
 
        if(to_spawn > 0){
            to_spawn--;
        }
        return;
    }
}

/* ===== Updating Enemies ===== */
void G2_Enemy_Update(void){
    if(to_spawn > 0){
        if(spawn_cd > 0){
            spawn_cd--;
        }else{
            Enemy_spawn();
            int16_t cd = 20 - wave;
            if(cd < 5){
                cd = 5;
            }
            spawn_cd = (uint8_t)cd;
        }
    }
 
    int16_t descend = (int16_t)G2_ENEMY_SPEED + (int16_t)(wave/4);
 
    for (int i = 0; i < G2_MAX_ENEMIES; ++i) {
        if(!enemies[i].active){
            continue;
        }
        enemies[i].y += descend;
        enemies[i].x += enemies[i].dx;
 
        if(enemies[i].x <= 0){
            enemies[i].x = 0;
            enemies[i].dx = 1;
        }else if(enemies[i].x >= (int16_t)(G2_LCD_W - G2_ENEMY_W)){
            enemies[i].x = (int16_t)(G2_LCD_W - G2_ENEMY_W);
            enemies[i].dx = -1;
        }
 
        //if they're no longer in the playable area, just despawn them
        if(enemies[i].y > (int16_t)G2_LCD_H){
            enemies[i].active = 0;
            continue;
        }
 
        if(enemies[i].cd > 0){
            enemies[i].cd--;
        }else{
            int16_t bx = enemies[i].x + (G2_ENEMY_W/2)-(G2_BULLET_W/2);
            int16_t by = enemies[i].y + G2_ENEMY_H;
            G2_Bullets_EnemyShoot(bx, by);
            enemies[i].cd = (uint8_t)(40 + Random_U16(40));
        }
    }
}

/* ===== Draw Enemies ===== */
void G2_Enemy_Draw(void){
    for(int i = 0; i < G2_MAX_ENEMIES; i++){
        if(!enemies[i].active){
            continue;
        }
        const uint8_t *sprite;
        uint8_t colour;
        if(enemies[i].type == 0){
            sprite = G2_SPR_ENEMY1;
            colour = G2_MAGENTA;
        }else{
            sprite = G2_SPR_ENEMY2;
            colour = G2_ORANGE;
        }
 
        LCD_Draw_Sprite_Colour((uint16_t)enemies[i].x, (uint16_t)enemies[i].y, G2_ENEMY_H, G2_ENEMY_W, sprite, colour);
 
        if(enemies[i].hp > 1){
            uint16_t bar_w = (uint16_t)(enemies[i].hp * (G2_ENEMY_W / 4));
            LCD_Draw_Rect((uint16_t)enemies[i].x, (uint16_t)(enemies[i].y - 3), bar_w, 2, G2_GREEN, 1);
        }
    }
}

/* ===== Collision ===== */
static void Enemy_kill(uint8_t i){
    G2_FX_Explosion(enemies[i].x + (G2_ENEMY_W / 2) - (G2_EXPLODE_W / 2), enemies[i].y + (G2_ENEMY_H / 2) - (G2_EXPLODE_H / 2));
    enemies[i].active = 0;
}
 
uint8_t G2_Enemy_CheckBulletHits(uint32_t *score_out, uint16_t wave){
    G2_Bullet *bullets = G2_Bullets_GetPlayerPool();
    uint8_t kills = 0;
 
    for(int i = 0; i < G2_MAX_ENEMIES; i++){
        if(!enemies[i].active){
            continue;
        }
        AABB ebox = {enemies[i].x, enemies[i].y, G2_ENEMY_W, G2_ENEMY_H };
 
        for(int j = 0; j < G2_BULLET_MAX; j++){
            if (!bullets[j].active){
                continue;
            }
            AABB bbox = {bullets[j].x, bullets[j].y, G2_BULLET_W, G2_BULLET_H };
 
            if(AABB_Collides(&bbox, &ebox)){
                bullets[j].active = 0;
 
                if(enemies[i].hp > 0){
                    enemies[i].hp--;
                }
                if(enemies[i].hp == 0){
                    Enemy_kill(i);
                    if(score_out != 0){
                        *score_out += 100 * (uint32_t)wave;
                    }
                    kills++;
                }
                break;
            }
        }
    }
    return kills;
}
 
uint8_t G2_Enemy_CheckPlayerCollision(AABB player_box){
    for(int i = 0; i < G2_MAX_ENEMIES; i++){
        if(!enemies[i].active){
            continue;
        }
        AABB ebox = {enemies[i].x, enemies[i].y, G2_ENEMY_W, G2_ENEMY_H };
        if(AABB_Collides(&player_box, &ebox)){
            Enemy_kill(i);
            return 1;
        }
    }
    return 0;
}