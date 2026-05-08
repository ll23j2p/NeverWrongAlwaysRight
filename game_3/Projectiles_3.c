#include "Game_3.h"
#include <stdlib.h>
#include <math.h>
#include "Char.h"

Ray rayArray[MAX_RAYS];

// helper funcs for adding some random angle to player shots
float RandomFloatMinus1To1(void)
{
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

float DegToRad(float degrees)
{
    return degrees * 3.14159265f / 180.0f;
}

// clears rayArray
void InitRays(void)
{
    for (int i = 0; i < MAX_RAYS; i++) {
        rayArray[i].active = 0;
        rayArray[i].x0 = 0;
        rayArray[i].y0 = 0;
        rayArray[i].x1 = 0;
        rayArray[i].y1 = 0;
        rayArray[i].lifetime = 0;
        rayArray[i].damage = 0;
    }
}

// checks for world collisions (add enemy collisions after adding enemies)
void CalculateRayEnd(float start_x, float start_y, float angle,
                     float *end_x, float *end_y)
{
    float dir_x = cosf(angle);
    float dir_y = sinf(angle);

    float last_valid_x = start_x;
    float last_valid_y = start_y;

    for (float dist = 0.0f; dist <= RAY_LENGTH; dist += RAY_STEP) {
        float test_x = start_x + dir_x * dist;
        float test_y = start_y + dir_y * dist;

        int tile_x = WorldToTileX(test_x);
        int tile_y = WorldToTileY(test_y);

        TileProps props = GetTileProps(tile_x, tile_y);

        if (props.solid) {
            break;
        }

        last_valid_x = test_x;
        last_valid_y = test_y;
    }

    *end_x = last_valid_x;
    *end_y = last_valid_y;
}

// adds rays to rayArray
void SpawnRay(float x0, float y0, float x1, float y1)
{
    for (int i = 0; i < MAX_RAYS; i++) {
        if (!rayArray[i].active) {
            rayArray[i].active = 1;
            rayArray[i].x0 = x0;
            rayArray[i].y0 = y0;
            rayArray[i].x1 = x1;
            rayArray[i].y1 = y1;
            rayArray[i].lifetime = 4;
            rayArray[i].damage = 1;
            return;
        }
    }

    // overwrite first ray if all slots are full
    rayArray[0].active = 1;
    rayArray[0].x0 = x0;
    rayArray[0].y0 = y0;
    rayArray[0].x1 = x1;
    rayArray[0].y1 = y1;
    rayArray[0].lifetime = 4;
    rayArray[0].damage = 1;
}

// "try shoot" function called by update player, currently we only shoot once per button press
void PlayerShoot(void)
{
    // Do not shoot if weapon is cooling down
    if (player.shoot_cooldown > 0) {
        return;
    }

    // Get weapon sprite / muzzle metadata
    int weapon_index = GetWeaponSpriteIndex();
    WeaponSprite weapon = weaponSprites[weapon_index];

    // Calculate muzzle position in world coordinates
    float muzzle_x = player.x + weapon.muzzle_x;
    float muzzle_y = player.y + weapon.muzzle_y;

    // Work out base firing angle from current aim/facing direction
    float base_angle_deg = 0.0f;

    if (player.aim > 0) {
        base_angle_deg = -90.0f;    // up
    }
    else if (player.aim < 0) {
        base_angle_deg = 90.0f;     // down
    }
    else if (player.facing > 0) {
        base_angle_deg = 0.0f;      // right
    }
    else {
        base_angle_deg = 180.0f;    // left
    }

    // Add small random spread
    float spread_deg = RandomFloatMinus1To1() * 4.0; // ± 4 degrees
    float final_angle_rad = DegToRad(base_angle_deg + spread_deg);

    // Calculate final ray endpoint, including wall collision
    float end_x;
    float end_y;

    CalculateRayEnd(muzzle_x, muzzle_y, final_angle_rad, &end_x, &end_y);

    // Store ray visual/effect
    SpawnRay(muzzle_x, muzzle_y, end_x, end_y);

    // Reset shoot cooldown
    player.shoot_cooldown = SHOOT_COOLDOWN_FRAMES;
}

// parse through rayArray and update ray animations
void UpdateRays(void)
{
    for (int i = 0; i < MAX_RAYS; i++) {
        if (!rayArray[i].active) {
            continue;
        }

        rayArray[i].lifetime--;

        if (rayArray[i].lifetime <= 0) {
            rayArray[i].active = 0;
        }
    }
}