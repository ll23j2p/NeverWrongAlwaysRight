#ifndef GAME1_CONFIG_H
#define GAME1_CONFIG_H

// All #define constants for Game 1 are centralised here so tuning
// the game (speed, sizes, screen layout) requires only one file.

// ===== TIMING =====
#define GAME1_FRAME_TIME_MS     30      // target ms per frame (~33 FPS)

// ===== PLAYER =====
#define PLAYER_SPRITE_SCALE     2       // each sprite pixel drawn as 2x2 on screen
#define PLAYER_SCREEN_Y         165     // fixed Y position of the player sprite
#define PLAYER_COLLISION_Y_MIN  150     // collision detection band -- top
#define PLAYER_COLLISION_Y_MAX  205     // collision detection band -- bottom

// ===== SCREEN / PERSPECTIVE =====
#define SCREEN_CENTRE_X         120     // horizontal midpoint of the LCD
#define HORIZON_Y               70      // Y where road vanishes (far end)
#define NEAR_Y                  210     // Y at the near edge (player end)
#define WORLD_MAX_Y             200     // world depth units -- 0 = far, 200 = near

// ===== LANE SPACING =====
// Lanes appear narrow at the horizon and wide at the near edge
#define FAR_LANE_SPACING        18
#define NEAR_LANE_SPACING       60

// ===== OBSTACLES =====
#define OBSTACLE_COUNT          4       //  max simultaneous obstacles on track
#define OBSTACLE_MIN_SIZE       8       // sprite size in pixels at the far end
#define OBSTACLE_MAX_SIZE       24      // sprite size in pixels at the near end
#define MARKER_SPACING          40      // world unit between road marker dashes

// ===== LCD PALETTE INDICES =====
// The LCD driver uses an indexed palette -- these map colour names to indices
#define LCD_TRANSPARENT_INDEX   255
#define LCD_BLACK_INDEX         0
#define LCD_WHITE_INDEX         1
#define LCD_RED_INDEX           2
#define LCD_GREEN_INDEX         3
#define LCD_BLUE_INDEX          4
#define LCD_YELLOW_INDEX        5
#define LCD_CYAN_INDEX          6
#define LCD_MAGENTA_INDEX       7
#define LCD_DARK_RED_INDEX      8
#define LCD_DARK_GREEN_INDEX    9
#define LCD_DARK_BLUE_INDEX     10
#define LCD_ORANGE_INDEX        11
#define LCD_PURPLE_INDEX        12
#define LCD_TEAL_INDEX          13
#define LCD_BROWN_INDEX         14
#define LCD_GREY_INDEX          15

#endif // GAME1_CONFIG_H
