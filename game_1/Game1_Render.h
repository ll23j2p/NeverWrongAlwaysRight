#ifndef GAME1_RENDER_H
#define GAME1_RENDER_H

// Game1_Render.h -- all drawing function prototypes for Game 1.
//
// Key concepts used in this module:
// - Render stage is fully separated from game logic -- nothing here modifies state
// - Indexed palette sprites: efficient and course-approved approach
// - Nearest-neighbour scaling: maps source pixels to target size via integer division
// - Road markers reuse Game1_ProjectToScreen so perspective scaling is automatic

#include <stdint.h>

// ===== FUNCTION PROTOTYPES =====
void Game1_DrawTrack(void);
void Game1_DrawObstacleSpriteAt(int16_t centre_x, int16_t centre_y, int16_t target_size);
void Game1_DrawEntities(void);
void Game1_DrawHUD(void);
void Game1_RenderFrame(void);

#endif // GAME1_RENDER_H
