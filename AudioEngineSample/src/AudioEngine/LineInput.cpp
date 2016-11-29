#include "AudioEngine/LineInput.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define RECORDRATE      44100
#define LOWLATENCY      /* Only really suited to Windows Vista/7.  XP/Directsound should avoid this. */

#ifdef LOWLATENCY
    #define LATENCY         ((RECORDRATE * 20) / 1000)   /* 20 = 20ms */
    #define DRIFTTHRESHOLD  ((RECORDRATE * 10) / 1000)   /* 10 = 10ms */
#else
    #define LATENCY         ((RECORDRATE * 50) / 1000)   /* 50 = 50ms */
    #define DRIFTTHRESHOLD  ((RECORDRATE * 25) / 1000)   /* 25 = 25ms */
#endif

LineInput::LineInput(AudioDeviceRef pDevice){
	setDeviceRecording(pDevice);
	
}
int LineInput::setup()
{
	adjustedlatency = LATENCY;
	datalength = 0;
	
	mSoundLevel = 1.f;
	mSoundSpeed = 1.f;
	lastrecordpos = 0.f;
	mLastRecording = 0;
	return 0;
}
int	LineInput::addDevicePlayback(AudioDeviceRef pDevice)
{
	int returnval = 0;
	FMOD::Sound* sound = 0;

	mSoundBufferMap[pDevice->getDeviceID()] = sound;
	mPlayBackDeviceMap[pDevice->getDeviceID()] = pDevice;
	recordLength = 0;

	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 1;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = RECORDRATE;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 5; /* 5 second buffer, doesnt really matter how big this is, but not too small of course. */
    
   mChannelSound			= 0;
   if ( pDevice->getSystem()->createSound(0, FMOD_2D | FMOD_SOFTWARE | FMOD_LOOP_NORMAL | FMOD_OPENUSER, &exinfo, &mSoundBufferMap[pDevice->getDeviceID()]) != FMOD_OK ) {
		console() << "Unable to load sound" << endl;
		return 1;
	}
   mSoundBufferMap[pDevice->getDeviceID()]->getLength(&soundlength, FMOD_TIMEUNIT_PCM);
   mSoundBufferLengthMap[pDevice->getDeviceID()] = recordLength;
	return returnval;
}
void	LineInput::setDeviceRecording(AudioDeviceRef pDevice){
	mRecordDevice = pDevice;
}
void LineInput::update()
{
	auto mSystem = mRecordDevice->getSystem();
	if(mLastRecording != mRecording){
		mLastRecording = mRecording;
		for ( auto buf : mSoundBufferMap ){
			mRecordDevice->getSystem()->recordStart(mRecordDevice->getDeviceID(), buf.second, true);
		}
	}
	unsigned int samplesrecorded = 0;
	unsigned int recordpos = 0, recorddelta;
	mSystem->getRecordPosition(mRecordDevice->getDeviceID(), &recordpos);   

	recorddelta = recordpos >= lastrecordpos ? recordpos - lastrecordpos : recordpos + recordLength - lastrecordpos;
	samplesrecorded += recorddelta;
	if (samplesrecorded >= adjustedlatency && !mChannelSound)
	{
		for(auto playbackdevice : mPlayBackDeviceMap){
			auto mSys = playbackdevice.second->getSystem();
			FMOD_RESULT result = mSys->playSound(FMOD_CHANNEL_FREE, mSoundBufferMap[playbackdevice.first], 0, &mChannelSound);
		}
	}

	if (mChannelSound && recorddelta)
	{
		unsigned int playrecorddelta;
		unsigned int playpos = 0;
		int adjusting = 0;
		float smootheddelta;
		float dampratio = 0.97f;
		static unsigned int minrecorddelta = (unsigned int)-1;
            
		/*
			If the record driver steps the position of the record cursor in larger increments than the user defined latency value, then we should
			increase our latency value to match.
		*/
		if (recorddelta < minrecorddelta)
		{
			minrecorddelta = recorddelta;
			if (adjustedlatency < recorddelta)
			{
				adjustedlatency = recorddelta;
			}
		}

		mChannelSound->getPosition(&playpos, FMOD_TIMEUNIT_PCM);
		playrecorddelta = recordpos >= playpos ? recordpos - playpos : recordpos + soundlength - playpos;
            
		/*
			Smooth total
		*/
		{
			static float total = 0;
                
			total = total * dampratio;
			total += playrecorddelta;
			smootheddelta = total * (1.0f - dampratio);
		}
           
		if (smootheddelta < adjustedlatency - DRIFTTHRESHOLD || playrecorddelta > soundlength / 2)   /* if play cursor is catching up to record (or passed it), slow playback down */
		{
			mChannelSound->setFrequency(RECORDRATE - (RECORDRATE / 50)); /* Decrease speed by 2% */
			adjusting = 1;
			//console()<<"adjusting speed down " << endl;
		}
		else if (playrecorddelta > adjustedlatency + DRIFTTHRESHOLD)   /* if play cursor is falling too far behind record, speed playback up */
		{
			mChannelSound->setFrequency(RECORDRATE + (RECORDRATE / 50)); /* Increase speed by 2% */
			adjusting = 2;
			//console()<<"adjusting speed up " << endl;
		}
		else
		{
			mChannelSound->setFrequency(RECORDRATE);          /* Otherwise set to normal rate */
			adjusting = 0;
		}
       
		//console()<<"REC "<<recordpos<<" (REC delta " << recorddelta<<") : PLAY "<<playpos<< ", PLAY/REC diff "<< (int)smootheddelta << " adjusting " << adjusting << endl;
	}
	lastrecordpos = recordpos;
	// Sample playback
	//mChannelSound->setFrequency( 44100.0f * mSoundSpeed );
	//mChannelSound->setVolume( mSoundLevel );

}
void	LineInput::setPlaySpeed(float pSpeed)
{ 
	mSoundSpeed = pSpeed; 
}
int LineInput::setRecording(bool pSwitch)
{
	mRecording = pSwitch;
	FMOD_RESULT result;
	 
	return 0;
}