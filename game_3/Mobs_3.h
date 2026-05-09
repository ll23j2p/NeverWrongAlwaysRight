#ifndef MOBS_3_H
#define MOBS_3_H

#include <stdint.h>
#include "Game_3.h"

#define MAX_ENEMIES 12

#define ENEMY_WIDTH 16
#define ENEMY_HEIGHT 16

#define ENEMY_HITBOX_OFFSET_X 2
#define ENEMY_HITBOX_OFFSET_Y 2
#define ENEMY_HITBOX_WIDTH 12
#define ENEMY_HITBOX_HEIGHT 14

#define ENEMY_HOP_FORCE 3.5f
#define ENEMY_HOP_SPEED 1.0f
#define ENEMY_GRAVITY 0.18f
#define ENEMY_HOP_COOLDOWN 30

typedef struct {
    uint8_t active;

    float x, y;
    float vx, vy;

    int health;
    int facing;       // -1 = left, 1 = right
    int grounded;

    int anim_row;
    int anim_col;

    int hop_timer;
    int hurt_timer;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];

void InitEnemies(void);
void SpawnMobs(float x, float y);
void UpdateMobs(void);

#endif