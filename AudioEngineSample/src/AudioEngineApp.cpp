
#include "AudioEngineApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void AudioEngineApp::setup()
{

	mAudioEngine = AudioEngine::create();
	int success = parseXML("AudioEngine_Settings.xml", "AudioEngine_Content.xml"  );
	mAudioEngine->setup();
	mAudioEngine->setRecording(true);
	mAudioEngine->loadSoundsSystem();
	mAudioEngine->registerEventListener( this, &AudioEngineApp::onSoundEvent );
	
	mFPSFont = Font( "Verdana", 23 );
	mFrameRate = 0.0f;

	 
	mPlaybackspeed = 1.0;
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 500 ) );
	mParams.addText( "PLAYBACK PARAMS" );
	mParams.addParam( "LineInput Playback Speed",&mPlaybackspeed,"min=0.1 max=2.0 step=0.001" );
	mParams.addSeparator();
	mParams.addText( "PLAYBACK DEVICES" );
	vector<string> playbackDevices = mAudioEngine->getPlaybackDeviceList(0);
	for( int32_t i = 0; i < playbackDevices.size(); i++ )
    {
		  mParams.addText( toString(i) + " : " + playbackDevices.at( i ));

    }
	mParams.addSeparator();
	mParams.addText( "RECORDING DEVICES" );
	vector<string> recDevices = mAudioEngine->getRecordingDeviceList(0);
	for( int32_t i = 0; i < recDevices.size(); i++ )
    {
		 mParams.addText( toString(i) + " : " + recDevices.at( i ));

    }
	mParams.addSeparator();
	mParams.addText( "MIDI DEVICES" );
	recDevices = mAudioEngine->getMidiDevices();
	for( int32_t i = 0; i < recDevices.size(); i++ )
    {
		 mParams.addText( toString(i) + " : " + recDevices.at( i ));

    }
	mParams.addSeparator();

	mAudioEngine->playSound("bd",4);
	// mParams.setOptions( "", "position='10 500'" );
	//mParams->addParam( "Full screen",				&mFullScreen,				"key=f" );
	
}
void AudioEngineApp::onSoundEvent( const AudioEngineEvent& event )
{
	switch ( event.sigType ){
		case MIDI_NOTE_EVENT :
			console()<< " AppModel::onSoundEvent MIDI_NOTE_EVENT " << event.sigType << " soundid " << event.soundID << " deviceID " << event.deviceID << endl;
			
			break;
		case SOUND_STOPPED_PLAYING :
			console()<< "onSoundEvent " << event.sigType << " soundid " << event.soundID << " deviceID " << event.deviceID << endl;
			int firstDeviceID = mAudioEngine->getDevice(0)->getDeviceID();
			if(event.deviceID == firstDeviceID ){
				if(event.soundID == "bd"){
					mAudioEngine->playSound("bd",4);
				}
			}
			break;
	}
}
void AudioEngineApp::parseSettings( XmlTree _root )
{
    XmlTree t = _root.getChild( "assetsPath" );
	t = _root.getChild( "audio" );
	XmlTree d = t.getChild("playback");
	for( auto & device : d.getChildren() )
    {
        Vec3f playerPos = Vec3f(device->getAttributeValue<float>( "playerposx" ),device->getAttributeValue<float>( "playerposy" ),device->getAttributeValue<float>( "playerposz" ));
		Vec3f playerLookAt = Vec3f(device->getAttributeValue<float>( "playerlookatx" ),device->getAttributeValue<float>( "playerlookaty" ),device->getAttributeValue<float>( "playerlookatz" ));
		mAudioEngine->addPlaybackDevice(device->getAttributeValue<int>( "id" ),playerPos,playerLookAt);
		
    }
	d = t.getChild("recording");
	for( auto & device : d.getChildren() )
    {
        mAudioEngine->addRecordingDevice(device->getAttributeValue<int>( "id" ));
    }
	d = t.getChild("midi");
	for( auto & device : d.getChildren() )
    {
        mAudioEngine->addMidiDevice(device->getAttributeValue<int>( "id" ));
    }
    //assetsPath = t.getValue< std::string >();

}
int AudioEngineApp::parseXML( string pAppFilePath, string pContentFilePath )
{
    mSettingsPath = "../settings/";
    mAssetsPath = "../assets/";

    XmlTree appTree;

    try
    {
        mSettingsFile = loadFile( mSettingsPath + pAppFilePath );
    }
    catch( XmlTree::Exception e )
    {
		console()<< "the settings file: " +  mSettingsPath + pAppFilePath + " can't be loaded." << endl;
        return -1;
    }

    try
    {
        appTree = XmlTree( mSettingsFile );
    }
    catch( XmlTree::Exception e )
    {
        console()<<"the settings file: " +  mSettingsPath + pAppFilePath + " can't be loaded." << endl;
        return -1;
    }

    XmlTree root;

    try
    {
        root = appTree.getChild( "AudioEngineSettings" );
    }
    catch( XmlTree::Exception e )
    {
        return -1;
    }

    try
    {
        parseSettings( root );
    }
    catch( XmlTree::Exception e )
    {
        console()<< "a problem occurred parsing the settings xml file. " + toString( e.what() ) << endl;
        return -1;
    }

    DataSourceRef contentFile = loadFile( mAssetsPath + pContentFilePath );
    XmlTree contentTree;

    try
    {
        contentTree = XmlTree( contentFile );
    }
    catch( XmlTree::Exception e )
    {
        console() << "the content xml file: " +  mAssetsPath + pContentFilePath + " can't be loaded." << endl;
        return -1;
    }

    try
    {
        root = contentTree.getChild( "AudioEngineContent" );
    }
    catch( XmlTree::Exception e )
    {
        console() << "the content file: " + mAssetsPath + pContentFilePath + " doesn't look like a LaunchPadContent file." << endl;
    }

    try
    {
        parseSounds( root );
    }
	catch( XmlTree::Exception e )
    {
        console() << "a problem occurred parsing the sound content xml file."<< endl;
        return -1;
    }
}
void AudioEngineApp::parseSounds( XmlTree _root )
{
    XmlTree t = _root.getChild( "Sounds" );

    for( auto & sound : t.getChildren() )
    {
        mAudioEngine->addSound( sound->getAttributeValue<string>( "id" ),sound->getAttributeValue< std::string >( "filename" ));
    }
}
void AudioEngineApp::mouseDown( MouseEvent event )
{
	
}
void AudioEngineApp::mouseUp( MouseEvent event )
{
	
}
void AudioEngineApp::mouseMove( MouseEvent event )
{
	
}
void AudioEngineApp::update()
{
	// Update frame rate
    mFrameRate = getAverageFps();
	mAudioEngine->update();
	mAudioEngine->setPlaybackSpeed(mPlaybackspeed);
	//mAudioEngine->getSoundItem("bd",0)->soundPos = Vec3f(sin(getElapsedSeconds())*4,0,0);
	mAudioEngine->getDevice(0)->setListenerPos(Vec3f(sin(getElapsedSeconds())*4,0,0),Vec3f(0,0,1));
	
}
void AudioEngineApp::resize()
{
	
}
// Prepare window
void AudioEngineApp::prepareSettings( Settings* settings )
{
    //settings->setWindowSize( kWindowSize.x, kWindowSize.y );
    settings->setFrameRate( 60.0f );
	//settings->setFullScreen(true);
}
// Handles key press
void AudioEngineApp::keyDown( KeyEvent event )
{
    
}
void AudioEngineApp::draw()
{
	 gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 


    gl::drawStringCentered( "fps: " + toString((int)mFrameRate), Vec2f( getWindowWidth() / 2, 2 ), ColorA::white(), mFPSFont );
    gl::enableAlphaBlending();
    gl::color( 1.0f, 1.0f, 1.0f );

	mParams.draw();
}

CINDER_APP_NATIVE( AudioEngineApp, RendererGl )
