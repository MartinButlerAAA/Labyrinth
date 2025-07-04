// The Sounds functions play sounds from header files, which have been taken from 48k sample 8-bit mono .wav files.
// The tool ProcessWav (see https://github.com/MartinButlerAAA/ProcessWav) was used to create the header files.
// WiiU sound functions developed to add sound to games learning from LiveSynthesisU.
#include <string.h>				// for memset
#include <sndcore2/core.h>		// for sound main functions.

#include "Sounds/SoundData.h"	// For the voice.h functions and structures and sound data.
#include "Sounds.h"				// For the sound API.

soundsel_t soundSel = NOCHANGE;		// integer for sound selected.

AXVoice* p_vceEffect;			// Pointer to the voice structure, returned by AXAcquireVoice. for effects.
AXVoice* p_vceBackground;		// Pointer to the voice structure, returned by AXAcquireVoice. for background music.

// The LiveSynthesisU program made changes to the voice settings in the callback function. I tried doing this outside of
// the callback function, but this didn't seem to work. I have therefore also made all changes to the voices in this
// callback function.
void axFrameCallback()
{
	AXVoiceOffsets voiceOffsets;	// AX Offsets with details about actual voice data used when making changes.

	// stopped
	if (soundSel == STOPBKGND)
	{ 
		AXSetVoiceState(p_vceBackground, 0);		// Set the state to stopped to stop the sound.
		putsoundSel(NOCHANGE);						// As action done set soundSel to do nothing.
	}
	// play Composition 1
	if (soundSel == STRTBKGND1)
	{
		AXSetVoiceState(p_vceBackground, 0);					// Set state to stopped while making changes.

		// Set the voice data to be passed into the voice to play the sound.
		voiceOffsets.dataType = 25; 							// type 25 is 8-bit sound data.
		voiceOffsets.loopingEnabled = 1;						// loop enabled for background music to keep looping round.
		voiceOffsets.loopOffset = 0;							// loop offset at very start.
		voiceOffsets.endOffset = (1919485 - 1);					// loop end at end of data (array size minus 1 see sound data header file).
		voiceOffsets.currentOffset = 0;							// set to start of data (although doesn't seem to be used).
		// Seems to be important that this is an unsigned char pointer (rather than void) for sound to actually work.
		voiceOffsets.data = (unsigned char*)Comp1Wave;	// pointer to the actual sound data.
		AXSetVoiceOffsets(p_vceBackground, &voiceOffsets);		// Pass the new voice data into the voice.

		AXSetVoiceState(p_vceBackground, 1);					// set state to playing.
		putsoundSel(NOCHANGE);
	}
	// play Composition 2
	if (soundSel == STRTBKGND2)
	{
		AXSetVoiceState(p_vceBackground, 0);					// Set state to stopped while making changes.

		// Set the voice data to be passed into the voice to play the sound.
		voiceOffsets.dataType = 25; 							// type 25 is 8-bit sound data.
		voiceOffsets.loopingEnabled = 1;						// loop enabled for background music to keep looping round.
		voiceOffsets.loopOffset = 0;							// loop offset at very start.
		voiceOffsets.endOffset = (1912756 - 1);					// loop end at end of data (array size minus 1 see sound data header file).
		voiceOffsets.currentOffset = 0;							// set to start of data (although doesn't seem to be used).
		// Seems to be important that this is an unsigned char pointer (rather than void) for sound to actually work.
		voiceOffsets.data = (unsigned char*)Comp2Wave;	// pointer to the actual sound data.
		AXSetVoiceOffsets(p_vceBackground, &voiceOffsets);		// Pass the new voice data into the voice.

		AXSetVoiceState(p_vceBackground, 1);					// set state to playing.
		putsoundSel(NOCHANGE);
	}
	// play Tennis Hit
	if (soundSel == MOVE)
	{
		AXSetVoiceState(p_vceEffect, 0);

		voiceOffsets.dataType = 25;
		voiceOffsets.loopingEnabled = 0;						// loop disabled for sound effects so they play once.
		voiceOffsets.loopOffset = 0;
		voiceOffsets.endOffset = (10306 - 1);					// loop end at end of data (array size minus 1)
		voiceOffsets.currentOffset = 0;
		voiceOffsets.data = (unsigned char*)TenHitWave;
		AXSetVoiceOffsets(p_vceEffect, &voiceOffsets);

		AXSetVoiceState(p_vceEffect, 1);
		putsoundSel(NOCHANGE);
	}
	// play Croak
	if (soundSel == TURN)
	{
		AXSetVoiceState(p_vceEffect, 0);

		voiceOffsets.dataType = 25;
		voiceOffsets.loopingEnabled = 0;
		voiceOffsets.loopOffset = 0;
		voiceOffsets.endOffset = (12260 - 1);
		voiceOffsets.currentOffset = 0;
		voiceOffsets.data = (unsigned char*)CroakWave;
		AXSetVoiceOffsets(p_vceEffect, &voiceOffsets);

		AXSetVoiceState(p_vceEffect, 1);
		putsoundSel(NOCHANGE);
	}
	// play Cheer
	if (soundSel == WIN)
	{
		AXSetVoiceState(p_vceEffect, 0);

		voiceOffsets.dataType = 25;
		voiceOffsets.loopingEnabled = 0;
		voiceOffsets.loopOffset = 0;
		voiceOffsets.endOffset = (239100 - 1);
		voiceOffsets.currentOffset = 0;
		voiceOffsets.data = (unsigned char*)CheerWave;
		AXSetVoiceOffsets(p_vceEffect, &voiceOffsets);

		AXSetVoiceState(p_vceEffect, 1);
		putsoundSel(NOCHANGE);
	}
	return;
}

// This is the sound initialisation borrowing heavily from LiveSynthesisU, but it has been extended to use two voices to 
// allow background music to be played as well as effects sounds. There seems to be a lot of initialisation. I have tried
// commenting out individual lines of code, but removing each line does stop something working, so it is all needed.
// This must use the function difinitions and data structures in SoundData.h NOT sndcore2/voice.h. I spent several painful days
// trying to get sound to work using voice.h.
void setupSound()
{
	AXInitParams axInitParams1;		// AX Init parameters for AX initialisation

	// The memset clears the structure to zeros. core.h shows that part of the structure as unused, but if it is not cleared
	// to zeros the intialisation intermittently fails to work.
	memset(&axInitParams1, 0, sizeof(AXInitParams));
	axInitParams1.renderer = AX_INIT_RENDERER_48KHZ;
	axInitParams1.pipeline = AX_INIT_PIPELINE_SINGLE;
	AXInitWithParams(&axInitParams1);

	AXRegisterFrameCallback((void*)axFrameCallback);			//callback function that will apply sound changes when commanded.

	// Two voices are acquired (there are 96 in total). One for sound effects and one to play background music.
	p_vceEffect = AXAcquireVoice(1, 0, 0);
	p_vceBackground = AXAcquireVoice(1, 0, 0);

	// Not exactly sure what the begin call does, but without it sound doesn't work.
	AXVoiceBegin(p_vceEffect);
	AXVoiceBegin(p_vceBackground);

	// Not exacly sure what setting this type to zero does, but have left it as sound is now working.
	AXSetVoiceType(p_vceEffect, 0);
	AXSetVoiceType(p_vceBackground, 0);

	// It is important to set the volume this way. I tried using the struture from voice.h, but could not get it to work.
	unsigned int vol = 0x80000000;
	AXSetVoiceVe(p_vceEffect, &vol);
	// Set background to half volume so effects stand out.
	vol = 0x40000000;
	AXSetVoiceVe(p_vceBackground, &vol);

	// The higher volume is set in the mix commands. Again I'm not quite sure what these do, but if the device mix commands are
	// commented out the correspnding voice stops.
	vol = 0x80000000;
	// This pass in structure is also taken directly from LiveSynthesisU.
	unsigned int mix[24];
	memset(mix, 0, sizeof(mix));
	mix[0] = vol;
	mix[4] = vol;
	// The second parameter is 0 for TV and 1 for game pad (DRC).
	// The mix must be set for each voice, for them to work. 
	AXSetVoiceDeviceMix(p_vceEffect, 0, 0, &mix);
	AXSetVoiceDeviceMix(p_vceBackground, 0, 0, &mix);
	// For now only the TV output will be set up.
	//AXSetVoiceDeviceMix(p_vceEffect, 1, 0, &mix);
	//AXSetVoiceDeviceMix(p_vceBackground, 1, 0, &mix);

	// Again not sure what the end call is doing, but the sound does not work if it is commented out.
	AXVoiceEnd(p_vceEffect);
	AXVoiceEnd(p_vceBackground);

	// Setting the source and source type is necessary. If not done the sound does not work.
	// In the LiveSynthesis example this was done in the callback function. As it is not going to be
	// changed it is done in the setup in this program.
	unsigned int ratioBits[4];
	ratioBits[0] = (unsigned int)(0x00010000 * ((float)48000 / (float)AXGetInputSamplesPerSec()));
	ratioBits[1] = 0;
	ratioBits[2] = 0;
	ratioBits[3] = 0;
	AXSetVoiceSrc(p_vceEffect, &ratioBits);
	AXSetVoiceSrc(p_vceBackground, &ratioBits);
	AXSetVoiceSrcType(p_vceEffect, 1);
	AXSetVoiceSrcType(p_vceBackground, 1);

	return;
}

// Tidy up sound as the program will exit.
void QuitSound()
{
	AXRegisterFrameCallback((void*)0);	//Assume this is necessary to stop sound running in background.
	AXQuit();							//Call sndcore2 quit function.
	return;
}

void putsoundSel(soundsel_t sndSel)
{
	soundSel = sndSel;	// Pass the sound selected into the sound processing.
	return;
}

