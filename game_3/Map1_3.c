#include "Game_3.h"

// --- Tilemap Data for Map 1 ---

const uint8_t tilemap1[MAP1_HEIGHT][MAP1_WIDTH] = {
    // Row 0
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    // Row 1
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    // Row 2
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    // Row 3
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    // Row 4
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 29, 30, 31},
    // Row 5
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 52, 53},
    // Row 6
    { 0,  0, 43,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 41, 42},
    // Row 7
    {11, 12, 54, 11,  0,  0,  0,  0, 43,  0,  0,  0,  0,  0,  0,  0,  0,  0, 52, 53},
    // Row 8
    {29, 30, 31, 32,  0,  0,  0, 11, 54, 11, 11,  0,  0,  0, 26,  0,  0,  0, 41, 42},
    // Row 9
    { 0, 41, 42,  0,  0,  0,  0, 29, 30, 31, 32,  0,  0,  0,  0,  0,  0,  0, 52, 53},
    // Row 10
    { 0, 41, 42,  0,  0,  0,  0,  0, 41, 42,  0,  0,  0,  0,  0,  0,  0, 15, 41, 42},
    // Row 11
    { 0, 52, 53,  0,  0,  0,  0,  0, 52, 53,  0,  0,  0, 11, 12,  0,  0,  0, 52, 53},
    // Row 12
    { 0, 41, 42,  0,  0,  0,  0,  0, 41, 42,  0,  0,  0, 16, 17,  0,  0,  0, 41, 42},
    // Row 13
    {36, 52, 53, 37, 38, 36, 39, 40, 52, 53, 36, 39, 40, 27, 28, 37, 38, 10, 52, 53},
    // Row 14 - ground tiles
    {47, 49, 51, 48, 49, 47, 50, 51, 49, 51, 47, 50, 51, 47, 51, 48, 51, 47, 51, 51},
};

// --- Map Functions: ---
/**
--- Get Tile Properties ---
*/
TileProps GetTileProps(int tile_x, int tile_y) {

  // If out of bounds, treat as solid
  if (tile_x < 0 || tile_x >= MAP1_WIDTH || 
      tile_y < 0 || tile_y >= MAP1_HEIGHT) {
    TileProps solid = {1, 0, 0};
    return solid;
  }

  uint8_t tile_id = tilemap1[tile_y][tile_x];
  return sunSet_tileProps[tile_id];

}

/**
--- Helper Functions to convert world coords (px) to tilemap coords (tiles) ---
*/
int WorldToTileX(float world_x) { return (int)(world_x / TILE_WIDTH); }
int WorldToTileY(float world_y) { return (int)(world_y / TILE_HEIGHT); }