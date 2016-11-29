#pragma once

#include "cinder/app/AppBasic.h"
#include "FMOD.hpp"
#include "AudioEngine/SoundItem.h"
#include "Events/Events.h"

class AudioDevice;
typedef boost::shared_ptr< AudioDevice > AudioDeviceRef;

class AudioDevice
{
    public:

		static AudioDeviceRef create( int pDeviceID )
		{ return (AudioDeviceRef)(new AudioDevice(pDeviceID)); }

		static AudioDeviceRef create()
		{ return (AudioDeviceRef)(new AudioDevice()); }

        AudioDevice(int pDeviceID = -1);
        void update();
		int setup();
		FMOD::System*	getSystem() { return mSystem; }
		int				getDeviceID(){ return mDeviceID; }
		int				playSound(SoundItemRef pSoundItem );
		void			setSoundPos(std::string soundID, ci::Vec3f pPos);
		int				createSound(SoundItemRef pSoundItem);
		void			setListenerPos(ci::Vec3f pPos,ci::Vec3f pLookAt);
		ci::Vec3f	getListenerPos(){
			return listenerPos;
		}
		SoundItemRef	getSoundItem(SoundItemRef pSoundItem);
		int				shutDown();

		 template< typename listener_t >
        ci::CallbackId					registerEventListener( listener_t* obj, void ( listener_t::*callback )( const AudioEngineEvent& ) )
        {
            return mAudioEngineEventCallbackMgr.registerCb( std::bind( callback, obj, std::placeholders::_1 ) );
        }
		
    private:
		void	updateListenerPos();
		FMOD_VECTOR pos;
		FMOD_VECTOR vel;

		FMOD_VECTOR listenerpos;

		ci::Vec3f listenerPos;
		ci::Vec3f lastListenerPos;
		ci::Vec3f listenerVel;
		ci::Vec3f listenerLookAt;
		ci::Vec3f forwardVec;

		 //external callback manager
        typedef ci::CallbackMgr< void( const AudioEngineEvent& ) >		AudioEngineEventCallbackMgr;
        AudioEngineEventCallbackMgr										mAudioEngineEventCallbackMgr;
		

        FMOD::System*				mSystem;
		std::vector<SoundItemRef>		mSounds;
		int							mDeviceID;
		void						createDsp( FMOD_DSP_TYPE type, FMOD::DSP** dsp );
		float						mSoundLevel;
		float						mSoundSpeed;

};
