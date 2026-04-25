#ifndef GAME_3_H
#define GAME_3_H

#include "Menu.h"
#include "Joystick.h"
#include "LCD.h"
#include "InputHandler.h"

#include "Map1_3.h"  // include tilemap data and GetTileProps function declaration

// --- Enums ---
typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_WIN
} GameState;

// --- Structs ---
typedef struct {
    float x, y;
    float vx, vy;
    int health;
    int facing;         // 1 = right, -1 = left
    int aim;            // currently 1 for up, -1 for down, 0 for horizontal - to be replaced with separate joystick aiming later
    int grounded;
    int shoot_cooldown;
    int anim_row;       // current spritesheet row: 0=Rface/normal, 1=Rface/up, 2=Rface/down, 3=Lface/normal, 4=Lface/up, 5=Lface/down
    int anim_col;       // current spritesheet col: 0=idle, 1=run1, 2=run2, 3=jump, 4=dead
    int anim_timer;     // counts frames before advancing animation
} Player;

typedef struct {
    float x, y;
    float vx, vy;
    int active;
    int damage;
    int owner;          // OWNER_PLAYER or OWNER_ENEMY
} Bullet;

typedef struct {
    float x, y;
    int health;
    int active;
    int type;           // enemy type enum
    int state;          // patrol/chase/attack etc
} Enemy;

typedef struct {
    float x, y;         // world position of top-left of screen
} Camera;

// --- Constants ---
#define DRAW_SCALE              2       // scale factor for rendering
#define SCREEN_WIDTH            120     // 240 / DRAW_SCALE
#define SCREEN_HEIGHT           120     // 240 / DRAW_SCALE

#define TILE_WIDTH              16      // std tile width in pixels (unscaled)
#define TILE_HEIGHT             16      // std tile height in pixels (unscaled)

#define PLAYER_HITBOX_WIDTH     10
#define PLAYER_HITBOX_HEIGHT    16
#define PLAYER_HITBOX_OFFSET_X  3       // starts 3px from left edge of sprite

#define PLAYER_SPEED            4.0f    // player velocity in pixels per frame
#define JUMP_FORCE              9.0f    // initial velocity applied when jumping

#define ANIM_FRAME_DURATION     6       // 6 gameframes per animation frame = 5 fps animations at 30 fps
#define SHOOT_COOLDOWN_FRAMES   10      // at 30 fps, this is about 1/3 second between shots
#define PLAYER_MAX_HEALTH       3       // three hits? maybe?

#define GRAVITY                 1   // gravity applied to player.vy per frame
#define MAX_BULLETS             32
#define MAX_ENEMIES             16
#define MAX_RAYS                8

// --- Global extern declarations ---
extern GameState gamestate;
extern Player    player;
extern Bullet    bullets[MAX_BULLETS];
extern Enemy     enemies[MAX_ENEMIES];
extern Camera    camera;

extern Joystick_cfg_t joystick_cfg;
extern Joystick_t     joystick_data;

// --- ISR flags (volatile!) ---
extern InputState current_input;  // Global input state updated by InputHandler.c, read by Game_3.c

// --- Function Declarations ---

// --- Game_3.c Functions ---
MenuState Game3_Run(void);

void ReadPlayerInput(void);

void InitialiseHardware(void);  // set up joystick, LCD, timers etc for game3 

void UpdateTitleScreen(void); 
void RenderTitleScreen(void); 

void UpdatePlayState(void); 
void RenderPlayState(void);

void UpdatePauseScreen(void); 
void RenderPauseScreen(void);

void UpdateGameOverScreen(void); 
void RenderGameOverScreen(void);

void UpdateWinScreen(void); 
void RenderWinScreen(void);

void CheckGameOverConditions(void);
void CheckWinConditions(void);

// --- Player_3.c Functions ---
void UpdatePlayer(void);

// --- Mobs_3.c Functions ---
void UpdateMobs(void);

// --- Projectiles_3.c Functions ---
void UpdateBullets(void);
void UpdateRays(void);

// --- Camera_3.c Functions ---
void UpdateCamera(void);

// --- Render_3.c Functions ---
void DrawBackground(const uint8_t *image, const uint16_t image_width, const uint16_t image_height);
void DrawTilemap(const uint8_t *tileset, const int tileset_cols);
void DrawMobs(void);
void DrawBullets(void);
void DrawRays(void);
void DrawPlayer(const uint8_t *spritesheet, const uint16_t spritesheet_width);
void DrawHUD(void);

#endif // GAME_3_H
