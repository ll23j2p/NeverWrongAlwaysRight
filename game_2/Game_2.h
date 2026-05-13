#ifndef GAME_2_H
#define GAME_2_H

#include <stdint.h>
#include "Menu.h"
#include "Joystick.h"
#include "Utils.h"

/**
 * @brief Game 2 - Student can implement their own game here
 * 
 * Placeholder for Student 2's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 * 
 * The menu system calls this function when Game 2 is selected.
 * The function runs its own loop and returns when the game exits.
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */

MenuState Game2_Run(void);

/* ===== Shared Constants ===== */

//Display consts
#define G2_LCD_W    240
#define G2_LCD_H    240
#define G2_FRAME_MS 40

//Player Stats + Info display consts
#define G2_HUD_H    16 //Heads up Display 
#define G2_TOP G2_HUD_H //Top of the playable area(below the HUD)
#define G2_BOT_MARG 4
#define G2_BOT (G2_LCD_H - G2_BOT_MARG) //Bottom of the playable area

//Colour consts
#define G2_BLACK    0
#define G2_WHITE    1
#define G2_RED  2
#define G2_GREEN    3
#define G2_BLUE 4
#define G2_ORANGE   5
#define G2_YELLOW   6
#define G2_PINK 7
#define G2_PURPLE   8
#define G2_NAVY 9
#define G2_GOLD 10
#define G2_VIOLET   11
#define G2_BROWN    12
#define G2_GREY 13
#define G2_CYAN 14
#define G2_MAGENTA  15

//Player consts
#define G2_PLAYER_W 16
#define G2_PLAYER_H 16
#define G2_PLAYER_SPEED 3
#define G2_PLAYER_HP_MAX    100
#define G2_PLAYER_INV   30 //no damage
#define G2_PLAYER_CD    10 //cooldown
#define G2_DEATH    45 //death animation 

//Damage values consts
#define G2_DMG_ENEMY_BUL 10 //from enemy bullet
#define G2_DMG_ENEMY_RAM  35 //from enemy ramming

//Bullets consts
#define G2_BULLET_W 4
#define G2_BULLET_H 8
#define G2_BULLET_PSPEED 8
#define G2_BULLET_ESPEED 4
#define G2_BULLET_MAX   6

//Enemy
#define G2_ENEMY_W  16
#define G2_ENEMY_H  16
#define G2_MAX_ENEMIES  6
#define G2_ENEMY_SPEED  2

//Wave System consts
#define G2_WAVE_BASE_COUNT  4
#define G2_WAVE_PER_WAVE    2
#define G2_WAVE_MAX_COUNT   16

//Effects consts
#define G2_EXPLODE_W    8
#define G2_EXPLODE_H    8
#define G2_EXPLOSION_FRAME  6
#define G2_EXPLOSION_TIMING 3
#define G2_EXPLOSION_MAX    8
#define G2_SPACEFIELD_COUNT 28
#define G2_SPEEDLINE_COUNT  6

/* ===== Shared types ===== */
typedef struct{
    int16_t x;
    int16_t y;
    uint8_t active;
}G2_Bullet;

/* ===== Sprite Data ===== */
extern const uint8_t G2_SPR_PLAYER  [256]; //16x16
extern const uint8_t G2_SPR_ENEMY1  [256]; //16x16
extern const uint8_t G2_SPR_ENEMY2  [256]; //16x16
extern const uint8_t G2_SPR_BULLET  [32]; //4x8
extern const uint8_t G2_SPR_EXPLODE1    [64]; //8x8
extern const uint8_t G2_SPR_EXPLODE2    [64]; //8x8

/* ===== Functions ===== */
//G2_player.c
void G2_Player_Init();
void G2_Player_Update(const Joystick_t *joy);
uint8_t G2_Player_Shoot(void);
void G2_Player_Draw(uint32_t count);
uint8_t G2_Player_GetHP(void);
uint8_t G2_Player_Damage(uint8_t oof);
uint8_t G2_Player_IsDead(void);
AABB G2_Player_GetAABB(void);
int16_t G2_Player_GetCentreX(void);
int16_t G2_Player_GetCentreY(void);
void G2_Player_Death_Start(void);
uint8_t G2_Player_Death(uint32_t count);

//G2_Bullets.c
void G2_Bullets_Init(void);
uint8_t G2_Bullets_PlayerShoot(int16_t x, int16_t y);
void G2_Bullets_UpdatePlayer(void);
void G2_Bullets_DrawPlayer(void);
G2_Bullet *G2_Bullets_GetPlayerPool(void);
uint8_t G2_Bullets_EnemyShoot(int16_t x, int16_t y);
void G2_Bullets_UpdateEnemy(void);
void G2_Bullets_DrawEnemy(void);
G2_Bullet *G2_Bullets_GetEnemyPool(void);

//G2_Enemy.c
void G2_Enemy_Init(void);
void G2_Enemy_NextWave(void);
uint16_t G2_Enemy_GetWave(void);
uint8_t G2_Enemy_WaveCleared(void);
void G2_Enemy_Update(void);
void G2_Enemy_Draw(void);
uint8_t G2_Enemy_CheckBulletHits(uint32_t *score_out, uint16_t wave);
uint8_t G2_Enemy_CheckPlayerCollision(AABB player_box);

//G2_fx.c
void G2_FX_SpacefieldInit(void);
void G2_FX_SpacefieldUpdateDraw(void);
void G2_FX_SpeedLinesInit(void);
void G2_FX_SpeedLinesUpdateDraw(void);
void G2_FX_ExplosionsInit(void);
void G2_FX_Explosion(int16_t x, int16_t y);
void G2_FX_ExplosionsUpdateDraw(void);

#endif // GAME_2_H
