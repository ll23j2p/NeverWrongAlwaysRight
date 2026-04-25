#include "Game_3.h"
#include "Map1_3.h"


void UpdateCamera(void) { // updates camera position based on player position
  
    // center camera on player:
  camera.x = player.x - (SCREEN_WIDTH / 2) + (TILE_WIDTH / 2);
  camera.y = player.y - (SCREEN_WIDTH / 2) + (TILE_WIDTH / 2);

  // clamp to map boundaries:
  if (camera.x < 0) {
    camera.x = 0;
  }
  if (camera.y < 0) {
    camera.y = 0;
  }
  if (camera.x > (MAP1_WIDTH * TILE_WIDTH) - SCREEN_WIDTH) {
    camera.x = (MAP1_WIDTH * TILE_WIDTH) - SCREEN_WIDTH;
  }
  if (camera.y > (MAP1_HEIGHT * TILE_WIDTH) - SCREEN_HEIGHT) {
    camera.y = (MAP1_HEIGHT * TILE_WIDTH) - SCREEN_HEIGHT;
  }
  
}