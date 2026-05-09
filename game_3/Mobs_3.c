#include "Game_3.h"

#include "Mobs_3.h"
#include "Map1_3.h"


Enemy enemies[MAX_ENEMIES];

void InitMobs(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }
}

void SpawnMobs(float x, float y)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = 1;
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].vx = 0.0f;
            enemies[i].vy = 0.0f;
            enemies[i].health = 3;
            enemies[i].facing = -1;
            enemies[i].grounded = 0;
            enemies[i].anim_row = 0;
            enemies[i].anim_col = 0;
            enemies[i].hop_timer = ENEMY_HOP_COOLDOWN;
            enemies[i].hurt_timer = 0;
            return;
        }
    }
}

void UpdateMobs(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {

        if (!enemies[i].active) {
            continue;
        }

        Enemy *enemy = &enemies[i];

        /* --- Damage / death check --- */
        if (enemy->health <= 0) {
            enemy->active = 0;
            continue;
        }

        if (enemy->hurt_timer > 0) {
            enemy->hurt_timer--;
        }

        /* --- Hop decision --- */
        if (enemy->grounded) {

            enemy->vx = 0.0f;

            if (enemy->hop_timer > 0) {
                enemy->hop_timer--;
            }

            if (enemy->hop_timer <= 0) {
                enemy->vy = -ENEMY_HOP_FORCE;
                enemy->vx = enemy->facing * ENEMY_HOP_SPEED;
                enemy->grounded = 0;
                enemy->hop_timer = ENEMY_HOP_COOLDOWN;
            }
        }

        /* --- Apply gravity --- */
        enemy->vy += ENEMY_GRAVITY;

        /* --- Horizontal movement and collision --- */
        enemy->x += enemy->vx;

        if (enemy->vx > 0.0f) {  // moving right

            int tile_x = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X + ENEMY_HITBOX_WIDTH - 1);
            int tile_y_top = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y);
            int tile_y_bot = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y + ENEMY_HITBOX_HEIGHT - 1);

            if (GetTileProps(tile_x, tile_y_top).solid ||
                GetTileProps(tile_x, tile_y_bot).solid) {

                enemy->x = (tile_x * TILE_WIDTH) - ENEMY_HITBOX_OFFSET_X - ENEMY_HITBOX_WIDTH;
                enemy->vx = 0.0f;
                enemy->facing = -1;
            }
        }
        else if (enemy->vx < 0.0f) {  // moving left

            int tile_x = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X);
            int tile_y_top = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y);
            int tile_y_bot = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y + ENEMY_HITBOX_HEIGHT - 1);

            if (GetTileProps(tile_x, tile_y_top).solid ||
                GetTileProps(tile_x, tile_y_bot).solid) {

                enemy->x = ((tile_x + 1) * TILE_WIDTH) - ENEMY_HITBOX_OFFSET_X;
                enemy->vx = 0.0f;
                enemy->facing = 1;
            }
        }

        /* --- Vertical movement and collision --- */
        enemy->y += enemy->vy;
        enemy->grounded = 0;

        if (enemy->vy >= 0.0f) {  // falling or standing

            int tile_y = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y + ENEMY_HITBOX_HEIGHT);

            int tile_x_left = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X);
            int tile_x_right = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X + ENEMY_HITBOX_WIDTH - 1);

            TileProps left_props = GetTileProps(tile_x_left, tile_y);
            TileProps right_props = GetTileProps(tile_x_right, tile_y);

            if (left_props.solid || right_props.solid ||
                left_props.platform || right_props.platform) {

                enemy->y = (tile_y * TILE_HEIGHT) - ENEMY_HITBOX_OFFSET_Y - ENEMY_HITBOX_HEIGHT;
                enemy->vy = 0.0f;
                enemy->grounded = 1;
            }
        }
        else {  // moving upward

            int tile_y = WorldToTileY(enemy->y + ENEMY_HITBOX_OFFSET_Y);

            int tile_x_left = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X);
            int tile_x_right = WorldToTileX(enemy->x + ENEMY_HITBOX_OFFSET_X + ENEMY_HITBOX_WIDTH - 1);

            if (GetTileProps(tile_x_left, tile_y).solid ||
                GetTileProps(tile_x_right, tile_y).solid) {

                enemy->y = ((tile_y + 1) * TILE_HEIGHT) - ENEMY_HITBOX_OFFSET_Y;
                enemy->vy = 0.0f;
            }
        }

        /* --- Basic animation state --- */
        enemy->anim_row = (enemy->facing < 0) ? 0 : 1;

        if (enemy->grounded) {
            enemy->anim_col = 0;      // idle
        }
        else if (enemy->vy < 0.0f) {
            enemy->anim_col = 2;      // jumping
        }
        else {
            enemy->anim_col = 1;      // falling / airborne
        }
    }
}