
// Maze functions to create random mazes then control the movement of the player through the maze.
// Including APIs to get the current maze view to support graphics display.

#include <stdlib.h>						// For rand() random numbers.
#include <stdio.h>						// For sprintf.
#include <stdbool.h>					// for booleans.
#include <unistd.h>						// For getcwd.
#include <string.h>						// For strcat and maybe other functions.

#include "Labyrinth.h"					// Header for maze access functions.
#include "Sounds.h"						// For sound effects.

#define MAZEADD	   4					// Added to maze size, so lower level mazes aren't too small.
#define MAZEBORDER 1					// Border around the maze to ensure all side corridors cannot run out of the maze.
#define BLKSIZE    3					// Size of maze building blocks.

										// Absolute maximum for maze game array surrounded by border.
#define MMAX	   (((MAXLEVEL + MAZEADD) * BLKSIZE) + MAZEBORDER + MAZEBORDER)

static char   gameMaze[MMAX][MMAX];		// Array containing the maze to be solved.
static unsigned int Msize;				// Maze size for the current maze. 

// These are initialised to be on the safe side but are set up in the game.
static unsigned int playerX = 2;		// Player X position in mazeArray
static unsigned int playerY = 2;		// Player Y postiion in mazeArray
static unsigned int playerD = 4;		// Player facing 1=North(up), 2=East(right), 3=South(down), 4=West(left)
static unsigned int level = 0;			// Game Level

// The four 3-way junction building blocks that random mazes are assembled from:
//  1    2    3    4
// # #  # #  ###  # #
//      #           #
// ###  # #  # #  # #
//
static char blk1[BLKSIZE][BLKSIZE] = { { '#', ' ', '#' }, { ' ', ' ', ' ' }, { '#', '#', '#' } };
static char blk2[BLKSIZE][BLKSIZE] = { { '#', ' ', '#' }, { '#', ' ', ' ' }, { '#', ' ', '#' } };
static char blk3[BLKSIZE][BLKSIZE] = { { '#', '#', '#' }, { ' ', ' ', ' ' }, { '#', ' ', '#' } };
static char blk4[BLKSIZE][BLKSIZE] = { { '#', ' ', '#' }, { ' ', ' ', '#' }, { '#', ' ', '#' } };

// Process to show a 2D representation of the full maze to aid program development on PC.
void twoDdisplay() 
{
	// Display the Maze array contents
	for (unsigned int y = 0; y < Msize; y++) 
	{
		for (unsigned int x = 0; x < Msize; x++) 
		{
			// Display the maze, but also show where the player is in the maze.
			// Note the array is vertical index first.
			if ((playerX == x) && (playerY == y)) 
			{ 
				// Player facing 1=North(up), 2=East(right), 3=South(down), 4=West(left)
				switch (playerD)
				{
					case 1:  { printf("^"); break; }
					case 2:  { printf(">"); break; }
					case 3:  { printf("_"); break; }
					default: { printf("<"); break; }
				}
			}
			else								  
			{ 
				printf("%c", gameMaze[y][x]); 
			}
		}
		printf("\n");
	}
	printf("\n");
}

// Accessor to get view along the corridor for the player in the direction they are facing.
// f is forward 0 to 8 along the corridor.
// s is side -1 for left side, 0 for along corridor, and 1 for right side.
char get3DView(int f, int s)
{
	unsigned int xi = 0;	// Working indices.
	unsigned int yi = 0;

	// If the view requested is outside of the valid range for looking along a corridor return a block.
	if ((f < 0) || (f > 8) || (s < -1) || (s > 1))
	{
		return '#';
	}

	// The view to be returned depends on which direction the player is facing.
	switch (playerD)
	{
		case 1: // North
		{
			// Working index is the offset from the current player position.
			xi = (int)playerX + s;
			yi = (int)playerY - f;
			break;
		}
		case 2:	// East
		{
			xi = (int)playerX + f;
			yi = (int)playerY + s;
			break;
		}
		case 3:	// South
		{
			xi = (int)playerX - s;
			yi = (int)playerY + f;
			break;
		}
		default:	// West
		{
			xi = (int)playerX - f;
			yi = (int)playerY - s;
			break;
		}
	}

	// If the player position is inside the maze find the correct character.
	if ((xi >= 0) && (xi < Msize) && (yi >= 0) && (yi < Msize))
	{
		// Return the character within the maze for the requested part of the corridor view.
		return gameMaze[yi][xi];
	}
	// For anything that is out of range report it as a block.
	return '#';
}

// Accessor to get any character from the maze including showing the player position and direction.
// This can be used to have a player aid in the Wii U game and helps with the PC development of the game.
char get2DView(int x, int y)
{
	unsigned int xi, yi;	// Working indices.

	// Working index is the offset from the current player position.
	xi = (int)playerX + x;
	yi = (int)playerY + y;

	// If the player position is inside the maze find the correct character.
	if ((xi >= 0) && (xi < Msize) && (yi >= 0) && (yi < Msize))
	{
		// If the indices are the player position, show the player direction.
		if ((y == 0) && (x == 0))
		{
			// Player facing 1=North(up), 2=East(right), 3=South(down), 4=West(left)
			// ASCII characters are used in the PC version. Really these need to be arrows.
			// On the Wii U a custom character set is used, so the ASCII characters actually display as arrows.
			switch (playerD)
			{
			case 1:  { return '^'; }
			case 2:  { return '>'; }
			case 3:  { return '_'; }	// Best ASCII character I can do for down.
			default: { return '<'; }
			}
		}
		else
		{
			// Return the character for the position within the maze requested.
			return gameMaze[yi][xi];
		}
	}
	// For anything that is out of range report it as a block.
	return '#';
}

// Move l for turn left, r for turn right and f for forward.
// Return 0 if move was not possible, 1 if move was performed and 2 if end of level.
unsigned int movePlayer(char move)
{
	// Player facing 1=North(up), 2=East(right), 3=South(down), 4=West(left)
	// Turn left.
	if ((move == 'l') || (move == 'L'))
	{
		playerD--;
		if (playerD < 1) { playerD = 4; }
		putsoundSel(TURN);
		return 1;
	}
	// Turn right.
	if ((move == 'r') || (move == 'R'))
	{
		playerD++;
		if (playerD > 4) { playerD = 1; }
		putsoundSel(TURN);
		return 1;
	}

	// Moving forward depends on direction.
	if ((move == 'f') || (move == 'F'))
	{
		//1=North(up)
		if ((playerD == 1) && (gameMaze[playerY - 1][playerX] == ' '))
		{
			playerY--;
			putsoundSel(MOVE);
			return 1;
		}
		//2=East(right)
		if ((playerD == 2) && (gameMaze[playerY][playerX + 1] == ' '))
		{
			playerX++;
			putsoundSel(MOVE);
			return 1;
		}
		//3=South(down)
		if ((playerD == 3) && (gameMaze[playerY + 1][playerX] == ' '))
		{
			playerY++;
			putsoundSel(MOVE);
			return 1;
		}
		//4=West(left)
		if ((playerD == 4) && (gameMaze[playerY][playerX - 1] == ' '))
		{
			playerX--;
			putsoundSel(MOVE);
			return 1;
		}
		// Check to see if the exit is found.
		if (((playerD == 1) && (gameMaze[playerY - 1][playerX] == 'E')) ||
			((playerD == 2) && (gameMaze[playerY][playerX + 1] == 'E')) ||
			((playerD == 3) && (gameMaze[playerY + 1][playerX] == 'E')) ||
			((playerD == 4) && (gameMaze[playerY][playerX - 1] == 'E')))
		{
			// Increment the level as the current maze is complete
			level++;
			if (level > MAXLEVEL) { level = MAXLEVEL; }

			// Overwrite the old data file with the new level, then return 2 to indicate the level is complete.
			writeLevel(level);
			putsoundSel(WIN);
			return 2;
		}
	}
	return 0;	// Move did not work.
}

// Return the current level to the game so that it can be displayed.
unsigned int getLevel()
{
	return level;
}

// Write the level to the data file, return false if this fails.
bool writeLevel(unsigned int newLevel)
{
	FILE* outFile;
	char fileName[1000];

	// Get the current working directory to find where connect is stored to update the game level.
	getcwd(fileName, sizeof(fileName));
	strcat(fileName, "/wiiu/apps/Labyrinth/level.txt");

	// If the level is less than 1 set it to 1.
	// If the level is above MAXLEVEL set to MAXLEVEL.
	level = newLevel;
	if (level < 1) { level = 1; }
	if (level > MAXLEVEL) { level = MAXLEVEL; }

	// Open the file, if the write to the file works return true.
	outFile = fopen(fileName, "wt");
	if (outFile != NULL)
	{
		fprintf(outFile, "%d \n", level);
		// To aid development and diagnostics, the names of the variable is also output to the text file.
		fprintf(outFile, "level \n");
		fclose(outFile);
		return true;
	}
	return false;	// If opening the file failed.
}

// Read the current level from the data file, if file not available then create one at level 1.
unsigned int readLevel()
{
	FILE* inFile;
	char fileName[1000];

	// Get the current working directory to find where connect is stored so that the weightings go with the game.
	getcwd(fileName, sizeof(fileName));
	strcat(fileName, "/wiiu/apps/Labyrinth/level.txt");

	// If the file is found, open it and get the current level.
	inFile = fopen(fileName, "rt");
	if (inFile != NULL)
	{
		fscanf(inFile, "%d \n", &level);
		fclose(inFile);

		// Limit any value read to a valid range, just in case.
		if (level < 1) { level = 1; }
		if (level > MAXLEVEL) { level = MAXLEVEL; }
		return level;
	}
	else
	{
		// If the file isn't there create it.
		writeLevel(1);
	}
	// Return the starting level if there was no file.
	return 1;
}

// Process to generate the Maze, size (and therefore difficulty) is set by the current game level.
void generateMaze() 
{
	unsigned int sel;	// Variable used for random numbers when constructing the maze.

	// The readLevel function ensures that the level is valid and less the MAXLEVEL;
	level = readLevel();

	// Set the maze size based on the level.
	Msize = ((level + MAZEADD) * BLKSIZE) + MAZEBORDER + MAZEBORDER;

	// Fill entire maze array with '#'s. Including all the area not currently used for the level.
	for (unsigned int y = 0; y < MMAX; y++)
	{
		for (unsigned int x = 0; x < MMAX; x++)
		{
			gameMaze[y][x] = '#';
		}
	}

	// Pick a random 3-way building block and put it in the top left corner of the maze.
	sel = rand() % 4;
	for (unsigned int y = 0; y < BLKSIZE; y++)
	{
		for (unsigned int x = 0; x < BLKSIZE; x++)
		{
			switch (sel)
			{
				case 0:  { gameMaze[MAZEBORDER + y][MAZEBORDER + x] = blk1[y][x]; break; }
				case 1:  { gameMaze[MAZEBORDER + y][MAZEBORDER + x] = blk2[y][x]; break; }
				case 2:  { gameMaze[MAZEBORDER + y][MAZEBORDER + x] = blk3[y][x]; break; }
				default: { gameMaze[MAZEBORDER + y][MAZEBORDER + x] = blk4[y][x]; break; }
			}
		}
	}

// The maze is built from left to right along rows and then down. Each time looking for unused blocks with open corridor
// connections that can be extended. This approach is repeated several times. It is likely on the first pass or two, there 
// will be blocks that were not used as there was no connecting corridor. Multiple passes allows that these blocks are also 
// used. Repeating by the level of the maze should always be enough to fill the gaps, if a few gaps are left it doesn't 
// matter, it will just make the maze a bit more difficult as they are likely fewer crossover points.
	for (unsigned int a = 0; a < (level + MAZEADD); a++)
	{
		// Loop through all of the maze building blocks to find blocks that have not yet been used.
		// The offset of border and blksize is to inspect the middle of the building block.
		// If it has not yet been used it will be '#' rather than a space.
		for (unsigned int y = MAZEBORDER + (BLKSIZE / 2); y < Msize; y = y + BLKSIZE)
		{
			for (unsigned int x = MAZEBORDER + (BLKSIZE / 2); x < Msize; x = x + BLKSIZE)
			{
				// If a building block that has not yet been used is found. Then see if it connects to an adjacent block.
				if (gameMaze[y][x] == '#')
				{
					// In each case one of three building blocks will be suitable, selected randomly.
					sel = rand() % 3;
					// If the connection is west then can only use blks 1, 3, 4.
					if (gameMaze[y][x - 2] == ' ')
					{
						for (unsigned int yi = 0; yi < BLKSIZE; yi++)
						{
							for (unsigned int xi = 0; xi < BLKSIZE; xi++)
							{
								switch (sel)
								{
								case 0:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk1[yi][xi]; break; }
								case 1:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk3[yi][xi]; break; }
								default: { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk4[yi][xi]; break; }
								}
							}
						}
					}
					// If the connection is east then can only use blks 1, 2, 3.
					if (gameMaze[y][x + 2] == ' ')
					{
						for (unsigned int yi = 0; yi < BLKSIZE; yi++)
						{
							for (unsigned int xi = 0; xi < BLKSIZE; xi++)
							{
								switch (sel)
								{
								case 0:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk1[yi][xi]; break; }
								case 1:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk2[yi][xi]; break; }
								default: { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk3[yi][xi]; break; }
								}
							}
						}
					}
					// If the connection is north then can only use blks 1, 2, 4.
					if (gameMaze[y - 2][x] == ' ')
					{
						for (unsigned int yi = 0; yi < BLKSIZE; yi++)
						{
							for (unsigned int xi = 0; xi < BLKSIZE; xi++)
							{
								switch (sel)
								{
								case 0:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk1[yi][xi]; break; }
								case 1:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk2[yi][xi]; break; }
								default: { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk4[yi][xi]; break; }
								}
							}
						}
					}
					// If the connection is south then can only use blks 2, 3, 4.
					if (gameMaze[y + 2][x] == ' ')
					{
						for (unsigned int yi = 0; yi < BLKSIZE; yi++)
						{
							for (unsigned int xi = 0; xi < BLKSIZE; xi++)
							{
								switch (sel)
								{
								case 0:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk2[yi][xi]; break; }
								case 1:  { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk3[yi][xi]; break; }
								default: { gameMaze[y - MAZEBORDER + yi][x - MAZEBORDER + xi] = blk4[yi][xi]; break; }
								}
							}
						}
					}
				}
			}
		}
	}

	// Set the player starting position to the top left of the maze and facing east.
	playerX = MAZEBORDER + (BLKSIZE / 2);
	playerY = MAZEBORDER + (BLKSIZE / 2);
	playerD = 2;

	// Position the exit searching in from the bottom right corner of the maze, until a space is found.
	// Find a position that is a side turning so that the exit isn't visible down a corridor.
	// Return once suitable exit added to maze, to avoid having more than one exit.
	for (unsigned int y = Msize - 1; y > 1; y--)
	{
		for (unsigned int x = Msize - 1; x > 1; x--)
		{
			// Surrounding blocks are checked to ensure that it is a side corridor and not visible along the length of a corridor.
			// South side corridor
			if ((gameMaze[y][x] == ' ') && (gameMaze[y][x - 1] == '#') && (gameMaze[y][x + 1] == '#') && (gameMaze[y + 1][x] == '#') && (gameMaze[y - 2][x] == '#'))
			{
				gameMaze[y][x] = 'E';
				return;
			}
			// North side corridor 
			if ((gameMaze[y][x] == ' ') && (gameMaze[y][x - 1] == '#') && (gameMaze[y][x + 1] == '#') && (gameMaze[y - 1][x] == '#') && (gameMaze[y + 2][x] == '#'))
			{
				gameMaze[y][x] = 'E';
				return;
			}
			// East side corridor
			if ((gameMaze[y][x] == ' ') && (gameMaze[y - 1][x] == '#') && (gameMaze[y + 1][x] == '#') && (gameMaze[y][x + 1] == '#') && (gameMaze[y][x - 2] == '#'))
			{
				gameMaze[y][x] = 'E';
				return;
			}
			// West side corridor
			if ((gameMaze[y][x] == ' ') && (gameMaze[y - 1][x] == '#') && (gameMaze[y + 1][x] == '#') && (gameMaze[y][x + 2] == '#') && (gameMaze[y][x - 1] == '#'))
			{
				gameMaze[y][x] = 'E';
				return;
			}
		}
	}
}
