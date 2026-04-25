#include "Game_3.h"
#include "Map1_3.h"
#include "Sunset_Tileset.h"

// --- Button redefinitions for easier reading ---
#define jump_button current_input.btn2_pressed
#define shoot_button current_input.btn3_pressed

void UpdatePlayer(void) {

  /**
  --- apply x joystick input to player velocity and facing (facing to be handled by 2nd joystick aim later) ---
  */
  if (joystick_data.coord_mapped.x > 0) {
    player.vx = PLAYER_SPEED;
    player.facing = 1;
  }
  else if (joystick_data.coord_mapped.x < 0) {
    player.vx = -PLAYER_SPEED;
    player.facing = -1;
  }
  else {
    player.vx = 0;  // no input = stop immediately (maybe add inertia later)
  }


  /**
  --- apply y joystick input to player aim (to be replaced with 2nd joystick aiming later) ---
  */
  if (joystick_data.coord_mapped.y > 0) {
    player.aim = 1;  // up
  }
  else if (joystick_data.coord_mapped.y < 0) {
    player.aim = -1;  // down
  }
  else {
    player.aim = 0;  // horizontal/no aim input
  }


  /**
  --- handle gravity and player jumping ---
  */
  player.vy += GRAVITY;  // always apply gravity

  if (jump_button) {
    jump_button = 0;  // clear flag - possibly change later to allow holding jump for variable height or double jumps
    if (player.grounded) { 
      player.vy = -JUMP_FORCE;
      player.grounded = 0;      // change grounded flag: will be set again when player collides with ground in world collision resolution blocks
      player.anim_timer = 0;    // reset so airborne animation timing starts fresh
      player.anim_col = 1;      // start jump with specific run frame
    }
  }


  /**
  --- update player x position and resolve world collisions ---
  */
  // apply x velocity
  player.x += player.vx;

  // determine which x direction we're moving in and resolve world collisions with left/right side of hitbox accordingly:

  if (player.vx > 0) {                                                                                // moving right
    int tile_x = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X + PLAYER_HITBOX_WIDTH - 1);  // right side of hitbox in tilemap coords
    int tile_y_top = WorldToTileY(player.y);                                                 // top of hitbox in tilemap coords
    int tile_y_bot = WorldToTileY(player.y + PLAYER_HITBOX_HEIGHT - 1);                      // bottom of hitbox in tilemap coords

    // if we're inside a solid tile, push player back to left edge of tile
    if (GetTileProps(tile_x, tile_y_top).solid || GetTileProps(tile_x, tile_y_bot).solid) {
      player.x = (tile_x * TILE_WIDTH) - PLAYER_HITBOX_OFFSET_X - PLAYER_HITBOX_WIDTH;
      player.vx = 0;
      
    }
  } 
  
  else if (player.vx < 0) {                                                                           // moving left
    int tile_x = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X);                            // left side of hitbox in tilemap coords
    int tile_y_top = WorldToTileY(player.y);                                                 // top of hitbox in tilemap coords
    int tile_y_bot = WorldToTileY(player.y + PLAYER_HITBOX_HEIGHT - 1);                      // bottom of hitbox in tilemap coords
    
    // if we're inside a solid tile, push player back to right edge of tile
    if (GetTileProps(tile_x, tile_y_top).solid || GetTileProps(tile_x, tile_y_bot).solid) {
      player.x = ((tile_x + 1) * TILE_WIDTH) - PLAYER_HITBOX_OFFSET_X;
      player.vx = 0;
    }

    // not working properly at left side of screen, temp fix:
    if (player.x < 0) { player.x = 0; }

  }


  /**
  --- update player y position and resolve world collisions ---
  */
  // apply y velocity
  player.y += player.vy;
  player.grounded = 0;  // assume !grounded until checked (i think this will handle walking off ledges?)

  // determine which y direction we're moving in and resolve world collisions with top/bottom of hitbox accordingly:

  if (player.vy > 0) {                                                                                       // moving down (check for solid & platform tiles)
    int tile_y = WorldToTileY(player.y + PLAYER_HITBOX_HEIGHT - 1);                                 // bottom of hitbox in tilemap coords
    int tile_x_left = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X);                              // left side of hitbox in tilemap coords
    int tile_x_right = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X + PLAYER_HITBOX_WIDTH - 1);   // right side of hitbox in tilemap coords

    TileProps left_tile = GetTileProps(tile_x_left, tile_y);    
    TileProps right_tile = GetTileProps(tile_x_right, tile_y);

    // if we're inside a solid/platform tile, snap player back to top of tile and set grounded to true
    if (left_tile.solid || right_tile.solid || left_tile.platform || right_tile.platform) {
      player.y = (tile_y * TILE_HEIGHT) - PLAYER_HITBOX_HEIGHT;
      player.vy = 0;
      player.grounded = 1;

    }
  }

  else if (player.vy < 0) {                                                                                  // moving up (check for solid tiles only)
    int tile_y = WorldToTileY(player.y);                                                            // top of hitbox in tilemap coords
    int tile_x_left = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X);                              // left side of hitbox in tilemap coords
    int tile_x_right = WorldToTileX(player.x + PLAYER_HITBOX_OFFSET_X + PLAYER_HITBOX_WIDTH - 1);   // right side of hitbox in tilemap coords

    // if we're inside a solid tile, snap player to bottom of tile
    if (GetTileProps(tile_x_left, tile_y).solid || GetTileProps(tile_x_right, tile_y).solid) {
      player.y = ((tile_y + 1) * TILE_HEIGHT);
      player.vy = 0;

    }
  }

  /**
  --- update anim_row and anim_col fields in player struct ---
  */
  // determine anim_row based on facing and aim:
  if (player.facing == 1) {                                   // facing right and...
    if (player.aim == 1)    player.anim_row = 1;              // looking up
    if (player.aim == -1)   player.anim_row = 2;              // looking down
    else                    player.anim_row = 0;              // looking straight
  } 
  else {                                                      // facing left and...
    if (player.aim == 1)    player.anim_row = 4;              // looking up
    if (player.aim == -1)   player.anim_row = 5;              // looking down
    else                    player.anim_row = 3;              // looking straight
  }

  static int cycle_index = 0;

  // determine anim_col from movement state:
  if (!player.grounded) {                   // airborne, briefly show col1 (run1) then col3 (jump)
    if (player.anim_timer < 4) {
      player.anim_col = 1;
      player.anim_timer++;
    } else {
      player.anim_col = 3;
    }
  }

  else if (player.vx > 0.1f || player.vx < -0.1f) {                // running, cycle thru cols 1, 0, 2, 0
    static const int run_cycle[] = {1, 0, 2, 0};
    player.anim_timer++;
    if (player.anim_timer >= ANIM_FRAME_DURATION) {
      player.anim_timer = 0;
      cycle_index = (cycle_index + 1) % 4;
      player.anim_col = run_cycle[cycle_index];
    }
  }

  else {                                    // idle
    player.anim_col = 0;
    player.anim_timer = 0;
    cycle_index = 0;  // reset running cycle
  }

  // handle shooting...

}