#pragma once

typedef enum
{
   SOUND_STOPPED_PLAYING,
   MIDI_NOTE_EVENT,
   PLAY_SOUND
} SIG_TYPE;

struct AudioEngineEvent
{
	SIG_TYPE		sigType;
    std::string		soundID;
    int				deviceID;
	int				noteNum;
};