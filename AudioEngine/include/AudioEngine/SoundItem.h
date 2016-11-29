#pragma once

#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/timeline.h"
#include "FMOD.hpp"
#include "fmod_errors.h"

class SoundItem;

typedef boost::shared_ptr< SoundItem > SoundItemRef;

static void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		cinder::app::console()<<"FMOD error! result "<< result << " error string: " << FMOD_ErrorString(result) << std::endl;
	}
}

class SoundItem {
	public:
		static SoundItemRef create()
		{
			return ( SoundItemRef )( new SoundItem() );
		}

		SoundItem(){};
		std::string id;
		std::string filename;
		FMOD::Sound      *sound;
		FMOD::Channel	*channel;
		ci::Anim<ci::Vec3f> soundPos;
		ci::Vec3f		lastSoundPos;
		ci::Vec3f		soundVel;
		bool playing;
};