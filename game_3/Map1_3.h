#ifndef MAP1_3_H
#define MAP1_3_H

#include "Sunset_Tileset.h"

#define MAP1_WIDTH  20  // 20 tiles wide (320px / 16px per tile)
#define MAP1_HEIGHT 15  // 15 tiles high (240px / 16px per tile)

extern const uint8_t tilemap1[MAP1_HEIGHT][MAP1_WIDTH];

// --- Function Declarations: ---
TileProps GetTileProps(int tile_x, int tile_y);
int WorldToTileX(float world_x);
int WorldToTileY(float world_y);

#endif // MAP1_3_H