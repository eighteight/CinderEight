#pragma once

#include "cinder/app/AppBasic.h"
#include "AudioEngine/AudioDevice.h"

#include "FMOD.hpp"



class LineInput;

typedef boost::shared_ptr< LineInput > LineInputRef;
class LineInput
{
    public:
		static LineInputRef create(AudioDeviceRef pDevice)
		{ return (LineInputRef)(new LineInput(pDevice)); }

        LineInput(AudioDeviceRef pDevice);
        void update();
		int setup();
		int setRecording(bool pSwitch);
		int		addDevicePlayback(AudioDeviceRef pDevice);
		void	setDeviceRecording(AudioDeviceRef pDevice);

		void	setPlaySpeed(float pSpeed);
		float	getPlaySpeed(){ return mSoundSpeed; }
		void	setVolume(float pVol){ mSoundLevel = pVol;}
		float	getVolume(){return mSoundLevel;}

    private:
        FMOD::Channel*				mChannelSound;
		FMOD_CREATESOUNDEXINFO		exinfo;
		unsigned int				adjustedlatency;
		unsigned int				datalength, soundlength;
		int							mRecordDeviceID;
		float						mSoundLevel;
		float						mSoundSpeed;
		bool						mRecording;
		bool						mLastRecording;
		unsigned int lastrecordpos;
		unsigned int recordLength;
		std::map<int, AudioDeviceRef>				mPlayBackDeviceMap;
		std::map<int, FMOD::Sound*>					mSoundBufferMap;
		std::map<unsigned int, float>						mSoundBufferLengthMap;
		AudioDeviceRef								mRecordDevice;
};
