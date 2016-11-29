#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h"

#include "events/Events.h"
#include "AudioEngine/AudioEngine.h"
#include "cinder/params/Params.h"


class AudioEngineApp : public cinder::app::AppNative
{
	public:
		void setup();
		void mouseDown( cinder::app::MouseEvent event );	
		void mouseUp( cinder::app::MouseEvent event );
		void mouseMove( cinder::app::MouseEvent event );
		void keyDown( cinder::app::KeyEvent event );
		void update();
		void draw();
		void resize();
        void								prepareSettings( ci::app::AppNative::Settings* settings );
		//external event listeners	
		void onSoundEvent( const AudioEngineEvent& event );
		void parseSettings( ci::XmlTree _root);
		int parseXML( std::string pAppFilePath, std::string pContentFilePath);
		void parseSounds( ci::XmlTree _root );
	private:
		
		ci::params::InterfaceGl				mParams;
        float								mFrameRate;
		ci::Font								mFPSFont;
		AudioEngineRef						mAudioEngine;
		std::vector<bool>						mPlaybackActive;
		float								mPlaybackspeed;

		ci::DataSourceRef		mSettingsFile;
        std::string				mSettingsPath;
        std::string				mBasePath;
        std::string				mAssetsPath;

};
