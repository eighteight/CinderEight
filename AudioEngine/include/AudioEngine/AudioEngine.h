#pragma once

#include "cinder/app/AppNative.h"
#include "Events/Events.h"
#include "boost/signals2.hpp"

#include "FMOD.hpp"
#include "AudioEngine/LineInput.h"
#include "AudioEngine/AudioDevice.h"
#include "AudioEngine/SoundItem.h"

#include "MidiIn.h"
#include "MidiMessage.h"
#include "MidiConstants.h"

class AudioEngine;

typedef boost::shared_ptr< AudioEngine > AudioEngineRef;

class AudioEngine
{
	#define SLIDER_NOTE 1
	#define BUTTON_ONE 48
	#define BUTTON_TWO 50
	#define BUTTON_THREE 52
    public:
        static AudioEngineRef create()
        {
            return ( AudioEngineRef )( new AudioEngine() );
        }

        AudioEngine();

        template< typename listener_t >
        ci::CallbackId					registerEventListener( listener_t* obj, void ( listener_t::*callback )( const AudioEngineEvent& ) )
        {
            return mAudioEngineEventCallbackMgr.registerCb( std::bind( callback, obj, std::placeholders::_1 ) );
        }

        void update();
        void setup();
          
		void	addSound(std::string pID, std::string pFile);
		//call this to load all sounds into every system for faster playback
		void	loadSoundsSystem();
		int		addPlaybackDevice(int pID, ci::Vec3f playerPos, ci::Vec3f playerLookAt);
		int		addRecordingDevice(int pID);
		int		addMidiDevice(int pID);
		int		setRecording(bool pSet);
		int		initializeInputs();
		void	setPlaybackSpeed(float pSpeed);
		void	setSoundPos(int pDeviceID, std::string soundID, ci::Vec3f pPos);
		ci::Vec3f	getListenerPos(int pDeviceID);
		SoundItemRef	getSoundItem(std::string pSoundID, int pID);
		AudioDeviceRef getDevice(int playerID){ return mDeviceMap[mDeviceIDs[playerID]]; }
		void	playSound(std::string pSoundID, int pDeviceID);
		std::vector<std::string>	getPlaybackDeviceList(int pID);
		std::vector<std::string>	getRecordingDeviceList(int pID);
		std::vector<std::string>	getMidiDevices();
        //external event listeners
        void onSoundEvent( const AudioEngineEvent& event );

        bool							isSetup()
        {
            return mIsSetup;
        }
        
    protected:
        //external callback manager
        typedef ci::CallbackMgr< void( const AudioEngineEvent& ) >		AudioEngineEventCallbackMgr;
        AudioEngineEventCallbackMgr										mAudioEngineEventCallbackMgr;
		bool mIsSetup;

		std::map<std::string, SoundItemRef>								mSoundMap;
		std::map<unsigned int, AudioDeviceRef>							mDeviceMap;
		std::map<unsigned int, AudioDeviceRef>							mRecordDeviceMap;
		std::map<unsigned int, FMOD::Channel*>							mChannelMap;
		std::vector<int>												mDeviceIDs;
		std::map<unsigned int, LineInputRef>							mInputMap;

		ci::midi::Input mMidiIns[3];
		int	numMidiPorts;
		ci::midi::Input midiPorts;
		void processMidiMessage(ci::midi::Message* message);
};

