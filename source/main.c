// Labyrinth game ported from PC to run on Wii U.
#include <stdio.h>				// For sprintf.
#include <stdbool.h>			// For booleans.

#include <coreinit/screen.h>	// For OSScreen.
#include <coreinit/thread.h>	// For Sleep.
#include <vpad/input.h>			// For the game pad inputs.
#include <whb/proc.h>			// For the loop and to do home button correctly.
#include <whb/log.h>			// ** Using the console logging features seems to help set up the screen output.
#include <whb/log_console.h>	// ** Found neeeded to keep these in the build for the program to display properly.

#include "Labyrinth.h"			// Header for the game processing.
#include "Draw.h"				// For graphics.
#include "Sounds.h"				// For game sound.

#define GREEN 0x00FE0000		// Green colour used t give green screen effect.

static unsigned int gameState = 0;	// 0 start, 1 playing, 2 new level, 3 end.
static char move = 0;				// The player move to pass into the game.
static unsigned int animate = 0;	// Count used for animation sequencing.
bool doMap = false;					// Flag to show if map display is enabled.
static unsigned int colour = 0;		// Used to fade text in.

// Put a border round the 3D display to make a neat edge.
void drawBorder()
{
	drawLine(XOFFSET, YOFFSET, XOFFSET + XDISPMAX, YOFFSET, GREEN);
	drawLine(XOFFSET, YOFFSET - 1, XOFFSET + XDISPMAX, YOFFSET - 1, GREEN);

	drawLine(XOFFSET, YOFFSET + YDISPMAX, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX, GREEN);
	drawLine(XOFFSET, YOFFSET + YDISPMAX + 1, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX + 1, GREEN);

	drawLine(XOFFSET, YOFFSET, XOFFSET, YOFFSET + YDISPMAX, GREEN);
	drawLine(XOFFSET - 1, YOFFSET, XOFFSET - 1, YOFFSET + YDISPMAX, GREEN);

	drawLine(XOFFSET + XDISPMAX, YOFFSET, XOFFSET + XDISPMAX, YOFFSET + YDISPMAX, GREEN);
	drawLine(XOFFSET + XDISPMAX + 1, YOFFSET, XOFFSET + XDISPMAX + 1, YOFFSET + YDISPMAX, GREEN);
}

// Put a border round the outside of the screen.
void drawOuterBorder()
{
	drawLine(20, 20, 1220, 20, GREEN);
	drawLine(20, 19, 1220, 19, GREEN);

	drawLine(20, 700, 1220, 700, GREEN);
	drawLine(20, 701, 1220, 701, GREEN);

	drawLine(20, 20, 20, 700, GREEN);
	drawLine(19, 20, 19, 700, GREEN);

	drawLine(1220, 20, 1220, 700, GREEN);
	drawLine(1221, 20, 1221, 700, GREEN);
}

// Put the instructions on the gamepad.
void vpadDisplay()
{
	char slevel[100] = "\0";	// String to display the current level.

	OSScreenClearBufferEx(SCREEN_DRC, 0x00000000u);	// Black background on gamepad.

	sprintf(slevel, "Level %i of %i", getLevel(), MAXLEVEL); // Current game level.

	// Put the text elements on the gamepad screen, showing which controller buttons to use and current level.
	// Use an old school green screen look.

	drawText("Labyrinth\0", GREEN, 3, 10, 50, SCREEN_DRC);

	drawText(slevel, GREEN, 3, 10, 100, SCREEN_DRC);

	drawText("Find your way out to reach the next level.\0", GREEN, 2, 10, 150, SCREEN_DRC);
	drawText("Use the left joycon or direction buttons.\0", GREEN, 2, 10, 180, SCREEN_DRC);
	drawText("Up    - forward\0", GREEN, 2, 10, 210, SCREEN_DRC);
	drawText("Left  - turn left\0", GREEN, 2, 10, 240, SCREEN_DRC);
	drawText("Right - turn right\0", GREEN, 2, 10, 270, SCREEN_DRC);
	drawText("X     - enable/disable map view\0", GREEN, 2, 10, 330, SCREEN_DRC);

	OSScreenFlipBuffersEx(SCREEN_DRC);
}

// Display a start screen to welcome the player ot the game.
void displayStartScreen()
{
	char slevel[100] = "\0";	// String to display the current level.

	sprintf(slevel, "Level %i ", getLevel()); // Current game level.

	drawText("Welcome to the Labyrinth!\0", colour, 3, 50, 50, SCREEN_TV);
	drawText("Press A to continue\0", colour, 3, 50, 100, SCREEN_TV);
	drawText("Press ZL and ZR to start again\0", colour, 3, 50, 150, SCREEN_TV);

	drawText(slevel, colour, 3, 50, 250, SCREEN_TV);

	// increase colour but limit to green to fade text in.
	colour = colour + 0x00040000u;
	if (colour > GREEN) { colour = GREEN;  }
}

// Display to show next level.
void displayLevelScreen()
{
	char slevel[100] = "\0";	// String to display the current level.

	sprintf(slevel, "Level %i", getLevel()); // Current game level.

	// Position the text in the middle of the screen, taking account of the text size.
	drawText(slevel, GREEN, 6, (1240 / 2) - (8*6*3), (720 / 2) - 24, SCREEN_TV);
}

// Display the reached the end of the game.
void displayEndScreen()
{
	drawText("Congratulations!\0", colour, 3, 50, 50, SCREEN_TV);
	drawText("You escaped the Labyrinth!\0", colour, 3, 50, 100, SCREEN_TV);
	drawText("Press B\0", colour, 3, 50, 150, SCREEN_TV);

	// increase colour but limit to green to fade text in.
	colour = colour + 0x00040000u;
	if (colour > GREEN) { colour = GREEN; }
}

// Function to display the current postion and area of 5 blocks in all directions.
// Also shows the direction the player is facing.
void displayMaze2D()
{
	char sline[12] = "\0";	// For assembling a line of the 2D maze display.

	// Display the maze around the player for 5 blocks in all directions.
	for (int y = 0; y <= 10; y++)
	{
		for (int x = 0; x <= 10; x++)
		{
			sline[x] = get2DView(x - 5, y - 5);
		}
		sline[11] = '\0';
		drawText(sline, GREEN, 3, 900, 100 + (y * 24), SCREEN_TV);
	}
}

// Function to display the view along a corridor and any side passages.
void displayMaze3D()
{
	float xoff = 0.0f;				// x offset used to slide image in for turning.
	float split = (1.0f / 6.0f);	// ratio for sides compared to central corridor.
	float x1 = 0.0f;				// working variables to draw straight lines.
	float x2 = 0.0f;
	float y1 = 0.0f;
	float y2 = 0.0f;

	// Shift the image to give the impression of turning for left and right.
	if ((move == 'r') && (animate == 1)) { xoff =  XDISPMAX * 0.8f; }
	if ((move == 'r') && (animate == 2)) { xoff =  XDISPMAX * 0.6f; }
	if ((move == 'r') && (animate == 3)) { xoff =  XDISPMAX * 0.4f; }
	if ((move == 'r') && (animate == 4)) { xoff =  XDISPMAX * 0.2f; }
	if ((move == 'l') && (animate == 1)) { xoff = -XDISPMAX * 0.8f; }
	if ((move == 'l') && (animate == 2)) { xoff = -XDISPMAX * 0.6f; }
	if ((move == 'l') && (animate == 3)) { xoff = -XDISPMAX * 0.4f; }
	if ((move == 'l') && (animate == 4)) { xoff = -XDISPMAX * 0.2f; }

	// Calculate the x and y coming in from the corners based on the screen split between corridor and sides.
	x2 = ((float)XDISPMAX * split);	// x working variable set to the corridor position.
	y2 = ((float)YDISPMAX * split);	// y working variable set to the corridor position.

	// Animation to move forard smoothly involves moving the x and y points back and bringing them back to the correct position in 5 steps.
	// This is because the move will have moved the player forward one whole block. 5 steps between complete moves is enough to give 
	// relatively smooth movement when viewed at 30 frames per second.
	// The reduction for perspective is 1/3 per block and the animation is in 5 steps, hence the use of 15ths.
	// As we have moved the x2 and y2 positions in for animation, the splt needs to be proprtionally reduced.
	// The display constants are integers, everything must be explicitly shown as floats to avoid the calculations being reduced to integers.
	if ((move == 'f') && (animate == 1)) 
	{ 
		x2 = ((float)XDISPMAX * split) + ((split * 8.0f / 15.0f) * (float)XDISPMAX);
		y2 = ((float)YDISPMAX * split) + ((split * 8.0f / 15.0f) * (float)YDISPMAX);
		split = split * (11.0f / 15.0f);
	}
	if ((move == 'f') && (animate == 2))
	{
		x2 = ((float)XDISPMAX * split) + ((split * 6.0f / 15.0f) * (float)XDISPMAX);
		y2 = ((float)YDISPMAX * split) + ((split * 6.0f / 15.0f) * (float)YDISPMAX);
		split = split * (12.0f / 15.0f);
	}
	if ((move == 'f') && (animate == 3))
	{
		x2 = ((float)XDISPMAX * split) + ((split * 4.0f / 15.0f) * (float)XDISPMAX);
		y2 = ((float)YDISPMAX * split) + ((split * 4.0f / 15.0f) * (float)YDISPMAX);
		split = split * (13.0f / 15.0f);
	}
	if ((move == 'f') && (animate == 4))
	{
		x2 = ((float)XDISPMAX * split) + ((split * 2.0f / 15.0f) * (float)XDISPMAX);
		y2 = ((float)YDISPMAX * split) + ((split * 2.0f / 15.0f) * (float)YDISPMAX);
		split = split * (14.0f / 15.0f);
	}

	// Construct the 3D display. Note that xoff is added to support rotation animation.
	for (int f = 0; f <= 8; f++)
	{ 
		// Display the exit in the centre of the screen if facing the exit.
		if (get3DView(f, 0) == 'E')
		{
			drawText("EXIT\0", GREEN, 6, XDISPCTR - 96 + XOFFSET + xoff, YDISPCTR - 24 + YOFFSET, SCREEN_TV);
			break;	// Stop at this point no need to show details behind the exit.
		}

		// Do centre of the corridor if blocked.
		if (get3DView(f, 0) == '#')
		{
			drawLineLtd(x1 + xoff, y1, XDISPMAX - x1 + xoff, y1, GREEN);						// horizontal top
			drawLineLtd(x1 + xoff, YDISPMAX - y1, XDISPMAX - x1 + xoff, YDISPMAX - y1, GREEN);	// horizontal bottom
			break;	// If the middle is a block no point in working further though the display.
		}

		// If the left side is blocked show the corridor continuing.
		if (get3DView(f, -1) == '#')
		{
			drawLineLtd(x1 + xoff, y1, x2 + xoff, y2, GREEN);									// diagonal left top
			drawLineLtd(x1 + xoff, YDISPMAX - y1, x2 + xoff, YDISPMAX - y2, GREEN);				// diagonal left bottom
		}
		else // Otherwise show a turning on the left.
		{
			drawLineLtd(x1 + xoff, y2, x2 + xoff, y2, GREEN);									// horizontal left top
			drawLineLtd(x1 + xoff, YDISPMAX - y2, x2 + xoff, YDISPMAX - y2, GREEN);				// horizontal left bottom
		}
		drawLineLtd(x2 + xoff, y2, x2 + xoff, YDISPMAX - y2, GREEN);							// vertical left (always needed)

		// If the right side is blocked show the corridor continuing.
		if (get3DView(f, 1) == '#')
		{
			drawLineLtd(XDISPMAX - x1 + xoff, y1, XDISPMAX - x2 + xoff, y2, GREEN);							// diagonal right top
			drawLineLtd(XDISPMAX - x1 + xoff, YDISPMAX - y1, XDISPMAX - x2 + xoff, YDISPMAX - y2, GREEN);	// diagonal right bottom
		}
		else // Otherwise show a turning on the right.
		{
			drawLineLtd(XDISPMAX - x1 + xoff, y2, XDISPMAX - x2 + xoff, y2, GREEN);							// horizontal right top
			drawLineLtd(XDISPMAX - x1 + xoff, YDISPMAX - y2, XDISPMAX - x2 + xoff, YDISPMAX - y2, GREEN);	// horizontal right bottom
		}
		drawLineLtd(XDISPMAX - x2 + xoff, y2, XDISPMAX - x2 + xoff, YDISPMAX - y2, GREEN);					// vertical right (always needed)

		split = split * 0.666667;		// reduce the split value by 1/3 each time to move along corridor into the distance.
		x1 = x2;						// make x1 start from the end of the previous line.
		x2 = x2 + (XDISPMAX * split);	// calculate the end for the next block.
		y1 = y2;						// make y1 the old y2.
		y2 = y2 + (YDISPMAX * split);	// calculate the end of the next block.
	}
}

void displays()     // Function to handle the displays.
{
	OSScreenClearBufferEx(SCREEN_TV,  0x00000000u);	// Black background on TV.

	vpadDisplay();	// Update the gamepad display

	drawOuterBorder(); // Put a border round the entire TV screen.

	// Call the correct display function based on game state.
	switch (gameState)
	{
		case 0: { displayStartScreen(); break; }
		case 2: { displayLevelScreen(); break; }
		case 3: { displayEndScreen(); break; }
		default: {
			drawBorder();							// Border for the 3D display area.
			if (doMap == true) { displayMaze2D(); } // Display 2D map if enabled.
			displayMaze3D();						// Display the 3D view of the current maze position.
		}
	}

	// Flip the screen buffer to show the new display.
    OSScreenFlipBuffersEx(SCREEN_TV);
	return;
}

// Do game state 0 for the start screen.
void doState0()
{
	VPADStatus status;			// Status returned for the gamepad button.
	VPADReadError error;		// Error from gamepad.

	VPADRead(VPAD_CHAN_0, &status, 1, &error);
	// Check that there were no errors, before processing the button pressed.
	if (error == VPAD_READ_SUCCESS)
	{
		// When button A press start a new level.
		if (status.hold & VPAD_BUTTON_A)
		{
			generateMaze();	// Create a random maze.

			// Set the tune to match the level.
			if (getLevel() % 2 == 1) { putsoundSel(STRTBKGND1); }
			else { putsoundSel(STRTBKGND2); }
			gameState = 1;	// Set state to playing.
		}
		if ((status.hold & VPAD_BUTTON_ZL) && (status.hold & VPAD_BUTTON_ZR))
		{
			writeLevel(1);								// Set level back to 1 before starting the game.
			OSSleepTicks(OSMillisecondsToTicks(50));	// Allow some time for file to write.
			generateMaze();								// Create a random maze.
			putsoundSel(STRTBKGND1);					// Start the music.
			gameState = 1;								// Set state to playing.
		}
	}
}

// Do game state 1 for playing the maze level.
void doState1()
{
	VPADStatus status;			// Status returned for the gamepad button.
	VPADReadError error;		// Error from gamepad.

	unsigned int ret;			// Return value from move player.

	if (animate == 0)
	{
		move = 0;
		// Get the VPAD button last pressed.
		VPADRead(VPAD_CHAN_0, &status, 1, &error);
		// Check that there were no errors, before processing the button pressed.
		if (error == VPAD_READ_SUCCESS)
		{
			if ((status.hold & VPAD_BUTTON_UP) || (status.hold & VPAD_STICK_L_EMULATION_UP)) { move = 'f'; }	// Player moves.
			if ((status.hold & VPAD_BUTTON_LEFT) || (status.hold & VPAD_STICK_L_EMULATION_LEFT)) { move = 'l'; }
			if ((status.hold & VPAD_BUTTON_RIGHT) || (status.hold & VPAD_STICK_L_EMULATION_RIGHT)) { move = 'r'; }

			if (status.trigger & VPAD_BUTTON_X) { doMap = !doMap; }	// Select map view.
		}

		ret = movePlayer(move);		// Send the move the game and check the response to the move.

		if (ret == 1)		// If move was a valid move start the animation.
		{
			animate = 1;
		}
		else if (ret == 2)	// If the move reached the exit, go to the new level state or end if reached the top level.
		{
			// If at the end go to end state, otherwise go to next level.
			if (getLevel() >= MAXLEVEL) { gameState = 3; colour = 0;  } // Set colour to 0 to fade text in.
			else { gameState = 2; }
		}
	}
	else // If we are animating increment the animation count for the next step.
	{
		animate++;
		if (animate == 5) { animate = 0; }	// Set back to 0 at the end of animation.
	}
}

// Do game state 2 for the new level screen.
void doState2()
{
	OSSleepTicks(OSMillisecondsToTicks(3000));		// Allow some time to be seen and heard.
	generateMaze();		// Create a new maze (slightly bigger for each level).

	// Alternate the background music for each level.
	if (getLevel() % 2 == 1) { putsoundSel(STRTBKGND1); }
	else { putsoundSel(STRTBKGND2); }

	gameState = 1;	// Go back to the playing state.
}

// Do game state 3 for the end of game screen.
void doState3()
{
	VPADStatus status;			// Status returned for the gamepad button.
	VPADReadError error;		// Error from gamepad.

	VPADRead(VPAD_CHAN_0, &status, 1, &error);
	// Check that there were no errors, before processing the button pressed.
	if (error == VPAD_READ_SUCCESS)
	{
		if (status.hold & VPAD_BUTTON_B) { gameState = 0; colour = 0; }  // Set colour to 0 to fade text in and go back to the start.
	}
}

// This is the main process and must be in the program at the start for the home button to operate correctly.
int main(int argc, char **argv) 
{
    WHBProcInit();
    WHBLogConsoleInit();	// Console Init seem to get the display to operate correctly so keep in the build.

	setupSound();

	readLevel();			// Get the level from the data file so that it is correct for the first screen.

	// There must be a main loop on WHBProc running, for the program to correctly operate with the home button.
	// Home pauses this loop and continues it if resume is selected. There must therefore be one main loop of processing in the main program.
    while (WHBProcIsRunning()) 
	{
		switch (gameState)
		{
			case 0:	// Start Screen.
			{
				doState0();
				break;
			}
			case 1:	// Playing a level
			{
				doState1();
				break;
			}
			case 2:	// New level
			{
				doState2();
				break;
			}
			default: // End
			{
				doState3();
				break;
			}
		}

		displays();	// Update the displays.
		OSSleepTicks(OSMillisecondsToTicks(30));		// Allow some time for moves to be seen.
    }

	QuitSound();

	// If we get out of the program clean up and exit.
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}