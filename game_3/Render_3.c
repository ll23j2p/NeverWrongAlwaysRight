#include "Game_3.h"
#include "LCD.h"
#include "Map1_3.h"
#include "Sunset_Backdrop.h"
#include "Sunset_Tileset.h"
#include <stdint.h>
#include <string.h>

// --- Function definitions ---


// OLD
// void DrawBackground(const uint8_t *image, const uint16_t image_width, const uint16_t image_height) {

//   for (int y = 0; y < SCREEN_HEIGHT; y++) {
//     for (int x = 0; x < SCREEN_WIDTH; x++) {

//       const uint8_t pixel = *((image + ((int)(y + camera.y)) * image_width) + (int)(x + camera.x));
//       // nb: easier to read as "pixel = image[((int)camera.y + y) * image_width + ((int)camera.x + x)]" but the pointer
//       // arithmetic version is how the render functions in the LCD library are written, so we'll keep it like this for now.

//       if (pixel == 255) { continue; }

//       for (int dy = 0; dy < DRAW_SCALE; dy++) {
//         for (int dx = 0; dx < DRAW_SCALE; dx++) {
         
//           LCD_Set_Pixel(x * DRAW_SCALE + dx, y * DRAW_SCALE + dy, pixel);

//         }
//       }

//     }
//   }

// }

// NEW
void DrawBackground(void) {
  LCD_Buffer_Blitz_Scaledx2(
    (const uint8_t*)Sunset_Backdrop, 
    (uint16_t)camera.x, 
    (uint16_t)camera.y, 
    SUNSET_BACKDROP_WIDTH, 
    SCREEN_WIDTH, 
    SCREEN_HEIGHT
  );
}

void DrawTilemap(const uint8_t *tileset, const int tileset_cols) {

  // get tilemap coords for the tile in top left corner of screen (camera.x & .y)
  int tile0_x = WorldToTileX(camera.x);
  int tile0_y = WorldToTileY(camera.y);
  int tile_x = tile0_x;  // "current" tiles in the loop
  int tile_y = tile0_y;

  // calculate screen/tile offset
  int offset_x = (int)camera.x % TILE_WIDTH;
  int offset_y = (int)camera.y % TILE_HEIGHT;

  for (int y = 0; y < SCREEN_HEIGHT; y += TILE_HEIGHT) {

    tile_x = tile0_x;  // this resets every time we start a row

    for (int x = 0; x < SCREEN_WIDTH; x += TILE_WIDTH) {

      uint8_t tile_id = tilemap1[tile_y][tile_x];  // get tile ID from level map array
      if (tile_id == 0) { tile_x++; continue; }  // if "empty" tile then increment tile_x and continue

      // source pixel in tileset to start reading from:
      int src_x = (tile_id % tileset_cols) * TILE_WIDTH;
      int src_y = (tile_id / tileset_cols) * TILE_HEIGHT;

      // how many pixels to draw from source pixel:
      int draw_cols = TILE_WIDTH;
      int draw_rows = TILE_HEIGHT;

      // how far off screen is current tile? 
      int clip_x = x - offset_x;
      int clip_y = y - offset_y;

      // left edge clipping:
      if (clip_x < 0) {
        src_x -= clip_x;  // skip into tile
        draw_cols += clip_x;  // draw less cols
      }

      // right edge clipping:
      if (clip_x + draw_cols > SCREEN_WIDTH) {
        draw_cols = SCREEN_WIDTH - clip_x;  // draw the cols that don't overlap the screen
      }

      // top edge clipping:
      if (clip_y < 0) {
        src_y -= clip_y;  // skip into tile
        draw_rows += clip_y;  // draw less rows
      }

      // bottom edge clipping:
      if (clip_y + draw_rows > SCREEN_HEIGHT) {
        draw_rows = SCREEN_HEIGHT - clip_y;  // draw the rows that don't overlap the screen
      }

      // now that tile has been clipped if required, draw the tile at scale starting at screen position of x*scale & y*scale
      for (int i = 0; i < draw_cols; i++) {
        for (int j = 0; j < draw_rows; j++) {

          const uint8_t pixel = tileset[(src_y + j) * (tileset_cols * TILE_WIDTH) + (src_x + i)];

          if (pixel == 255) { continue; }

          for (int dy = 0; dy < DRAW_SCALE; dy++) {
            for (int dx = 0; dx < DRAW_SCALE; dx++) {

              int draw_x = (clip_x < 0 ? 0 : clip_x) + i;  // if the clip vals are off screen then clamp to 0, size of clipped tiles are modified above.
              int draw_y = (clip_y < 0 ? 0 : clip_y) + j;

              LCD_Set_Pixel(draw_x * DRAW_SCALE + dx, draw_y * DRAW_SCALE + dy, pixel); 

            }
          }
        }
      }

      tile_x++;
    }

    tile_y++;
  }
}

void DrawMobs(void) {
    // Implementation for drawing mobs
}

void DrawBullets(void) {
    // Implementation for drawing bullets
}

void DrawRays(void) {
    // Implementation for drawing rays
}

void DrawPlayer(const uint8_t *spritesheet, const uint16_t spritesheet_width) {

  // screen position of player sprite:
  int draw_x = (player.x - camera.x);
  int draw_y = (player.y - camera.y);
  
  // start pixels on sprite sheet:
  int src_x = player.anim_col * TILE_WIDTH;
  int src_y = player.anim_row * TILE_HEIGHT;

  for (int y = 0; y < TILE_HEIGHT; y++) {
    for (int x = 0; x < TILE_WIDTH; x++) {

      const uint8_t pixel = spritesheet[(src_y + y) * spritesheet_width + (src_x + x)];

      if (pixel == 255) { continue; }

      for (int dy = 0; dy < DRAW_SCALE; dy++) {
        for (int dx = 0; dx < DRAW_SCALE; dx++) {

          LCD_Set_Pixel((draw_x + x) * DRAW_SCALE + dx, (draw_y + y) * DRAW_SCALE + dy, pixel);

        }
      }

    }
  }

}


void DrawHUD(void) {
    // Implementation for drawing HUD
}