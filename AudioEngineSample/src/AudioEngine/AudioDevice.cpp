#include "AudioEngine/AudioDevice.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const float DISTANCEFACTOR = 1.0f;          // Units per meter.  I.e feet would = 3.28.  centimeters would = 100.


AudioDevice::AudioDevice(int pDeviceID){
	mDeviceID = pDeviceID;
	console()<<"AudioDevice::AudioDevice(int pDeviceID)" << pDeviceID << endl;
	//setListenerPos(Vec3f(0,0,0),Vec3f(0,1,0));
}

int AudioDevice::setup()
{
	mSystem					= 0;
	
	// Basic FMOD system initialization
	FMOD::System_Create( &mSystem );
	if ( mSystem == 0 ) {
		console() << "Unable to create system" << endl;
		return -1;
	}
	FMOD_OUTPUTTYPE type;
	mSystem->getOutput(&type);
	console() <<"mDeviceID " << mDeviceID << " output type " << type << endl;
	if(mDeviceID != -1)
		mSystem->setDriver(mDeviceID);
	if ( mSystem->init( 32, FMOD_INIT_NORMAL, 0 ) != FMOD_OK ) {
		console() << "Unable to initialize system. deviceid " << mDeviceID << endl;
		return -1;
	}
	console()<<"system " << mDeviceID << " inited " << endl;
	mSystem->set3DSettings(1.0, DISTANCEFACTOR, 1.0f); 
	return 0;
}
int AudioDevice::playSound(SoundItemRef pSoundItem ){
	int soundindex = 0;
	for(int i=0; i<mSounds.size(); i++){
		if(mSounds[i]->id == pSoundItem->id){
			soundindex = i;
			break;
		}
	}
	FMOD_VECTOR  sPos;
	sPos.x = mSounds[soundindex]->soundPos().x;
	sPos.y = mSounds[soundindex]->soundPos().y;
	sPos.z = mSounds[soundindex]->soundPos().z;
	FMOD_VECTOR  sVel;
	sVel.x = mSounds[soundindex]->soundVel.x;
	sVel.y = mSounds[soundindex]->soundVel.y;
	sVel.z = mSounds[soundindex]->soundVel.z;
	mSounds[soundindex]->channel->set3DAttributes(&sPos,&sVel);
	FMOD::Channel         *channel = 0;
	int result = mSystem->playSound(FMOD_CHANNEL_FREE, mSounds[soundindex]->sound, true, &mSounds[soundindex]->channel);
	//ERRCHECK((FMOD_RESULT)result);
	//console()<<"playSound id " << mSounds[soundindex].id << " result " << result << endl;
	if(result == 0){
		mSounds[soundindex]->playing = true;
		
	}
	return result;

}
SoundItemRef	AudioDevice::getSoundItem(SoundItemRef pSoundItem){
	SoundItemRef returnItem;
	for(int i=0; i<mSounds.size(); i++){
		if(mSounds[i]->id == pSoundItem->id){
			returnItem = mSounds[i];
			break;
		}
	}
	return returnItem;
}

void AudioDevice::setSoundPos(std::string soundID, Vec3f pPos){
	for ( int i = 0; i< mSounds.size(); i++ ){
		if(mSounds[i]->id == soundID){
			mSounds[i]->lastSoundPos = mSounds[i]->soundPos;
			mSounds[i]->soundPos = pPos;
			mSounds[i]->soundVel = pPos - mSounds[i]->lastSoundPos;
			console()<<"setSoundPos id " << mSounds[i]->id << " soundpos " << mSounds[i]->soundPos << endl;
			break;
		}
	}
}
void AudioDevice::updateListenerPos(){

	listenerVel = listenerPos - lastListenerPos;
	lastListenerPos = listenerPos;

	FMOD_VECTOR  lPos;
	FMOD_VECTOR	 lVel;

	lPos.x = listenerPos.x;
	lPos.y = listenerPos.y;
	lPos.z = listenerPos.z;

	lVel.x = listenerVel.x;
	lVel.y = listenerVel.y;
	lVel.z = listenerVel.z;

	
	FMOD_VECTOR forward;
	forward.x = forwardVec.x;
	forward.y = forwardVec.y;
	forward.z = forwardVec.z;
	
    FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };

	mSystem->set3DListenerAttributes(0, &lPos, &lVel, &forward, &up); 
}
void AudioDevice::setListenerPos(ci::Vec3f pPos, ci::Vec3f pLookAt)
{ 
	listenerLookAt = pLookAt;
	lastListenerPos = listenerPos;
	listenerPos = pPos;
	listenerVel = listenerPos - lastListenerPos;
	forwardVec = (listenerLookAt - listenerPos).normalized(); 
	
}
int AudioDevice::createSound(SoundItemRef pSoundItem){
	SoundItemRef sound = SoundItem::create();
	mSounds.push_back(sound);
	SoundItemRef snd = mSounds[mSounds.size()-1];
	mSounds[mSounds.size()-1]->filename = pSoundItem->filename;
	mSounds[mSounds.size()-1]->id = pSoundItem->id;
	mSounds[mSounds.size()-1]->sound = 0;
	mSounds[mSounds.size()-1]->channel = 0;
	mSounds[mSounds.size()-1]->playing = false;
	mSounds[mSounds.size()-1]->soundPos = Vec3f(0.f,0.f,0.f);
	mSounds[mSounds.size()-1]->soundVel = Vec3f(0.f,0.f,0.f);

	int result = mSystem->createSound(("../assets/sounds/"+mSounds[mSounds.size()-1]->filename).c_str(), FMOD_3D, 0, &mSounds[mSounds.size()-1]->sound);
	ERRCHECK((FMOD_RESULT)result);
	mSounds[mSounds.size()-1]->sound->set3DMinMaxDistance(.5f * DISTANCEFACTOR, 100.0f * DISTANCEFACTOR);
	console()<<"createSound deviceID " << mDeviceID << " result " << result << endl;
	
	return result;
}
int AudioDevice::shutDown()
{
	for ( auto snd : mSounds ){
		int result = snd->sound->release();
		ERRCHECK((FMOD_RESULT)result);
	}
	int result = mSystem->close();
	ERRCHECK((FMOD_RESULT)result);
	result = mSystem->release();
	ERRCHECK((FMOD_RESULT)result);
	return result;
}
void AudioDevice::update()
{
	for ( int i = 0; i< mSounds.size(); i++ ){
		bool prevPlay = mSounds[i]->playing;
		int result = mSounds[i]->channel->isPlaying(&mSounds[i]->playing);
		//update position of sound
		//if(mSounds[i]->channel){
			//source is fixed so velocity is zero
			FMOD_VECTOR  sPos;
			sPos.x = mSounds[i]->soundPos().x;
			sPos.y = mSounds[i]->soundPos().y;
			sPos.z = mSounds[i]->soundPos().z;
			FMOD_VECTOR  sVel;
			sVel.x = mSounds[i]->soundVel.x;
			sVel.y = mSounds[i]->soundVel.y;
			sVel.z = mSounds[i]->soundVel.z;
			mSounds[i]->channel->set3DAttributes(&sPos,&sVel);
			mSounds[i]->channel->setPaused(false);
			updateListenerPos();
		//}
		if(prevPlay != mSounds[i]->playing){
			if(mSounds[i]->playing == false){
				AudioEngineEvent event;
				event.sigType = SOUND_STOPPED_PLAYING;
				event.soundID = mSounds[i]->id;
				event.deviceID = mDeviceID;
				mAudioEngineEventCallbackMgr.call( event );
			}
			else {
				mSounds[i]->channel->setPaused(false);
			}
		}
		
	}
	mSystem->update();
}
 