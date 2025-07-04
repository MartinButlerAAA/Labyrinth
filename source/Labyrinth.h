#pragma once

// Maze APIs to support graphical display and game operation.

#include <stdbool.h>					// for booleans.

#define MAXLEVEL   25					// Highest possible game level.

// Access functions for the maze generation and play to support the Labyrinth game.

void generateMaze(void);				// Create a new maze.

unsigned int movePlayer(char move);		// Move player, return if move was successful and if level complete.
										// 0 move not possible, 1 moved OK, 2 end of level (found the exit).

char get2DView(int x, int y);			// Accessor to get the view around the player. Out of range requests are reported as blocks.
char get3DView(int f, int s);			// Accessor to get view along a corridor to support 3D display. Out of range requests are reported as blocks.

unsigned int getLevel(void);			// Return the current game level.

unsigned int readLevel(void);			// Read the game level from the data file.
										// This is only needed right at the start of the game to show the current level.

bool writeLevel(unsigned int newLevel);	// Write the newLevel to the data file, return false if this fails. 
										// Can be called to set the level back to 1 to go back to easy mode.

void twoDdisplay(void);					// Display the entire maze (only used during PC development).
