#include "AudioEngine/AudioEngine.h"
#include "cinder/Utilities.h"
#include "Resources.h"


using namespace std;
using namespace cinder;
using namespace app;




AudioEngine::AudioEngine( ) : mIsSetup( false ) , numMidiPorts(0)
{

}
void AudioEngine::setup()
{
	initializeInputs();
    mIsSetup = true;
}
void AudioEngine::update()
{
	for( auto device : mDeviceMap ){
		device.second->update();
	}
	for( auto input : mInputMap ){
		input.second->update();
	}
	for( int i=0; i<3;i++ ){
		while (mMidiIns[i].hasWaitingMessages()) {
			midi::Message message;
			mMidiIns[i].getNextMessage(&message);
		
			processMidiMessage(&message);
		
		}
	}
} 
SoundItemRef	AudioEngine::getSoundItem(std::string pSoundID, int pID)
{ 

	int deviceID = mDeviceIDs[pID];
	SoundItemRef item = mDeviceMap[deviceID]->getSoundItem(mSoundMap[pSoundID]);
	return item;
}
void AudioEngine::processMidiMessage(midi::Message* message){
	//console() << "midi port: " << message->port << " ch: " << message->channel << " status: " << message->status;
	//console() << " byteOne: " << message->byteOne << " byteTwo: " << message->byteTwo << std::endl;
	
	switch (message->status) {
		case MIDI_CONTROL_CHANGE:
			if (message->byteOne == SLIDER_NOTE){
				float sliderValue = message->byteTwo / 127.0f;
			}
			break;
		case MIDI_NOTE_ON:
			int noteValue = (message->byteOne - 47)/ 2 ;
			//console()<< "note " << noteValue << endl;
			AudioEngineEvent event;
			event.sigType = MIDI_NOTE_EVENT;
			event.deviceID = message->port;
			event.noteNum = noteValue;
			mAudioEngineEventCallbackMgr.call( event );
			break;
	}
}
void AudioEngine::setSoundPos(int pDeviceID, std::string soundID, ci::Vec3f pPos){
	if(pDeviceID == 4){
		for( auto device : mDeviceMap ){
			device.second->setSoundPos(soundID, pPos);
		}
	}
	else {
		int deviceID = mDeviceIDs[pDeviceID];
		mDeviceMap[deviceID]->setSoundPos(soundID, pPos);
	}
}
int AudioEngine::addPlaybackDevice(int pID, Vec3f playerPos, Vec3f playerLookAt){
	int returnval = 0;
	AudioDeviceRef newDevice = AudioDevice::create(pID);
	newDevice->setListenerPos(playerPos,playerLookAt);
	returnval = newDevice->setup();
	mDeviceMap[pID] = newDevice;
	mDeviceIDs.push_back(pID);
	newDevice->registerEventListener( this, &AudioEngine::onSoundEvent );
	return returnval;
}
int AudioEngine::addRecordingDevice(int pID){
	int returnval = 0;
	AudioDeviceRef newDevice = AudioDevice::create(pID);
	returnval = newDevice->setup();
	mRecordDeviceMap[pID] = newDevice;
	return returnval;
}
int AudioEngine::initializeInputs(){
	int returnval = 0;
	for( auto device : mRecordDeviceMap ){
		LineInputRef input = LineInput::create(device.second);
		returnval = input->setup();
		input->setPlaySpeed(1.f);
		input->setVolume(1.f);
		for ( auto playbackdevice : mDeviceMap ){
			returnval = input->addDevicePlayback(playbackdevice.second);
		}
		mInputMap[device.first] = (input);
	}
	return returnval;
}
int AudioEngine::setRecording(bool pSet){
	int returnval = 0;
	for( auto input : mInputMap ){
		returnval = input.second->setRecording(pSet);
	}
	return returnval;
}
int AudioEngine::addMidiDevice(int pID){
	
	if (mMidiIns[numMidiPorts].getNumPorts() > 0){
		mMidiIns[numMidiPorts].openPort(pID);
		console() << "Opening MIDI port "<<pID << std::endl;
	}else {
		console() << "No MIDI Ports found!!!!" << std::endl;
	}
	numMidiPorts++;
	return 0;
}
ci::Vec3f AudioEngine::getListenerPos(int pDeviceID){
	int deviceID = mDeviceIDs[pDeviceID];
	return mDeviceMap[deviceID]->getListenerPos();
}
void AudioEngine::playSound(string pSoundID, int pDeviceID){
	
	if(pDeviceID == 4){
		for( auto device : mDeviceMap ){
			int result = device.second->playSound(mSoundMap[pSoundID]);
			ERRCHECK((FMOD_RESULT)result);
			console()<<"AudioEngine::playSound deviceID " << device.second->getDeviceID() << " result " << result << endl;
		}
	}
	else {
		int deviceID = mDeviceIDs[pDeviceID];
		int result = mDeviceMap[deviceID]->playSound(mSoundMap[pSoundID]);
		ERRCHECK((FMOD_RESULT)result);
		console()<<"AudioEngine::playSound deviceID " << mDeviceMap[deviceID]->getDeviceID() << " result " << result << endl;
	}

}
vector<std::string>	AudioEngine::getPlaybackDeviceList(int pID)
{
	vector<std::string> returnList;
	int numdrivers;

	AudioDeviceRef newDevice = AudioDevice::create();
	newDevice->setup();

	int res = newDevice->getSystem()->getNumDrivers(&numdrivers);
	//initalize recording device
	for (int count=0; count < numdrivers; count++)
	{
		char name[256];

		newDevice->getSystem()->getDriverInfo(count, name, 256, 0);
		returnList.push_back(string(name));
		console() <<"playback driver " +toString(count) + " name " + string(name) << endl;

	}
	return returnList;
}
vector<std::string>	AudioEngine::getRecordingDeviceList(int pID)
{
	vector<std::string> returnList;
	int numdrivers;

	AudioDeviceRef newDevice = AudioDevice::create();
	newDevice->setup();

	int res = newDevice->getSystem()->getRecordNumDrivers(&numdrivers);
	//initalize recording device
	for (int count=0; count < numdrivers; count++)
	{
		char name[256];

		newDevice->getSystem()->getRecordDriverInfo(count, name, 256, 0);
		returnList.push_back(string(name));
		console() <<"rec driver " +toString(count) + " name " + string(name) << endl;

	}
	return returnList;
}
void	AudioEngine::setPlaybackSpeed( float pSpeed){
	for( auto input : mInputMap ){
		input.second->setPlaySpeed(pSpeed);
	}
}
vector<string> AudioEngine::getMidiDevices(){
	return midiPorts.listPorts();
	
}
void AudioEngine::addSound(string pID, string pFile){
	SoundItemRef newSound = SoundItem::create();
	newSound->id = pID;
	newSound->filename = pFile;
	mSoundMap[pID] = newSound;
}
void AudioEngine::loadSoundsSystem()
{
	for( auto snd : mSoundMap ){
		for( auto device : mDeviceMap ){
			device.second->createSound(snd.second);
		}
	}
}

//external callback listeners

void AudioEngine::onSoundEvent( const AudioEngineEvent& event )
{
	mAudioEngineEventCallbackMgr.call( event );
}