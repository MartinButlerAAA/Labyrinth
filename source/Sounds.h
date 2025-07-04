#pragma once
// The function interface to play sounds.

// A value from enum SOUNDSEL.
typedef uint32_t soundsel_t;

enum SOUNDSEL
{
	NOCHANGE   = 0,
	STOPBKGND  = 1,
	STRTBKGND1 = 2,
	STRTBKGND2 = 3,
	MOVE       = 4,
	TURN       = 5,
	WIN        = 6,
};

// Call this to set up sounds once, before attempting to call putsoundSel.
extern void setupSound();

// Call this with one of the above SOUNDSEL for that sound action.
extern void putsoundSel(soundsel_t sndSel);

// Call this once before exiting to close down sound.
extern void QuitSound();
