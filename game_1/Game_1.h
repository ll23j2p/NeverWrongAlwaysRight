#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"

/**
 * @brief Game 1 setup function
 *
 * Initialises the starting state for Game 1.
 */
void Game1_Init(void);

/**
 * @brief Game 1 reset function
 *
 * Resets the game state variables back to their starting values.
 */
void Game1_Reset(void);

/**
 * @brief Game 1 input handling function
 *
 * Reads input and checks for exit conditions.
 */
void Game1_HandleInput(void);

/**
 * @brief Game 1 lane update function
 *
 * Updates the player position so it moves between lane positions.
 */
void Game1_UpdatePlayerLane(void);

/**
 * @brief Game 1 - Student can implement their own game here
 *
 * Placeholder for Student 1's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 *
 * The menu system calls this function when Game 1 is selected.
 * The function runs its own loop and returns when the game exits.
 *
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */
MenuState Game1_Run(void);

#endif // GAME_1_H