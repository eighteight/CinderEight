#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timer.h"
#include "cinder/Xml.h"

#include "CinderFFmpeg.h"
#include "cinderSyphon.h"
#include "cinder/DataSource.h"
#include "Resources.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"

#include "UdpClient.h"

#include "ofxARDrone.h"
using namespace ci;
using namespace ci::app;
using namespace ph;
using namespace std;

class ardroneApp : public AppNative {
public:
	void setup();
	
	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );

	void update();
	void draw();

	void loadMovieFile( const fs::path &path );
    void prepareSettings( Settings *settings );
    void resize();
private:
	gl::Texture			mFrameTexture;
	ffmpeg::MovieGlRef	mMovie;
    std::string mUrlString;
    gl::Fbo    renderFbo;
    gl::Texture mInfoTexture;
    syphonServer mSyphonServer;
    void setInfo(const std::string& path, const std::string& errorMessage = "");
    void setupMovie();

    bool mIsEqualMovieSize;
    
    //udp
    UdpClientRef				mClient;
    std::string					mHost;
    int32_t						mPort;
    std::string					mRequest;
    UdpSessionRef				mSession;
    void						write();
    
    void						onConnect( UdpSessionRef session );
    void						onError( std::string err, size_t bytesTransferred );
    void						onWrite( size_t bytesTransferred );
    
    ofxARDrone::Drone drone;                // the main big daddy class
    ofxARDrone::Simulator droneSimulator;   // for displaying on screen (OPTIONAL)
    
    bool keys[65535];
    bool doPause;

    void drawBar(int x, int y, int w, int h, float t, bool doVertical);
    float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp);
    void drawDrone();
    
};

void ardroneApp::prepareSettings( Settings * settings )
{
    try {
        const XmlTree xml = XmlTree( loadResource(RES_SETTINGS_FILE) ).getChild("settingsXml");
        mUrlString = xml.getChild("url").getValue();

        bool ontop = xml.getChild("ontop").getValue<int>(1);
        settings->setAlwaysOnTop(ontop);
        mIsEqualMovieSize = xml.getChild("sizeEqualsMovieSize").getValue<bool>(1);
        if (mIsEqualMovieSize) {
            settings->setResizable(false);
        } else {
            int height = xml.getChild("height").getValue<int>(400);
            int width = xml.getChild("width").getValue<int>(600);
            bool isResizable = xml.getChild("resizable").getValue<bool>();
            settings->setResizable( isResizable );
            settings->setWindowSize( width, height );
        }
    } catch(...){
        cout<<"Could not read xml settings, using default url"<<endl;
        mUrlString = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
        //mUrlString = "tcp://192.168.1.1:5555";
    }
}

void ardroneApp::setup()
{
    setupMovie();
    
    mSyphonServer.setName("ffmpeg-to-syphon");
    getWindow()->setTitle("eight_io: ffmpeg-to-syphon");

    renderFbo = gl::Fbo( getWindowWidth(), getWindowHeight() );
    
    drone.connect();
    
    // setup command history lengths for debugging and dumping onscreen (OPTIONAL)
    drone.controller.commandHistory.setMaxLength(30);
    drone.dataReceiver.commandHistory.setMaxLength(30);
    
    // setup the simulator so we have a display in the viewport (OPTIONAL)
    droneSimulator.setup(&drone);


}

void ardroneApp::setupMovie()
{
    std::string errorMessage = "";
    try {
        if (mMovie)
            mMovie.reset();
        mMovie = ffmpeg::MovieGl::create(DataSourceUrl::create(Url(mUrlString)));
        mMovie->play();
        if (mIsEqualMovieSize) getWindow()->setSize(mMovie->getWidth(), mMovie->getHeight());
    } catch (exception& e ) {
        errorMessage = string("Unable to load the movie. ") + string(e.what());
        console() << errorMessage << std::endl;
    }
    
    setInfo(mUrlString, errorMessage);
}

void ardroneApp::resize()
{
    renderFbo = gl::Fbo( getWindowWidth(), getWindowHeight() );
}

void ardroneApp::keyDown( KeyEvent event )
{
    char key = event.getChar();
	if( event.getChar() == 'o' ) {
		fs::path moviePath = getOpenFilePath();
		if( ! moviePath.empty() )
			loadMovieFile( moviePath );
    } else 	if( event.getChar() == 'r' ) {
        setupMovie();
    }
    
    
    switch(key) {
        case '1': drone.controller.exitBootstrap(); break;
        case '2': drone.controller.sendAck(); break;
        case '3': drone.dataReceiver.sendDummyPacket(); break;
        case '0': drone.controller.resetCommunicationWatchdog(); break;
            
        case 't': drone.controller.takeOff(!drone.state.isTakingOff(), 3000); break;
        case 'l': drone.controller.land(!drone.state.isLanding(), 3000); break;
        case 'c': drone.controller.calibrateHorizontal(!drone.state.isCalibratingHorizontal(), 3000); break;
        case 'm': drone.controller.calibrateMagnetometer(!drone.state.isCalibratingMagnetometer(), 3000); break;
        case 'p': doPause ^= true; break;
            
        case 'e': drone.controller.emergency(0); break;
        case 'E': drone.controller.emergency(1); break;
            
        case 'r': droneSimulator.reset(); break;
        case 'R': drone.resetSequenceNumber(); break;
            
        case 'h':
            drone.controller.commandHistory.setMaxLength(drone.controller.commandHistory.getMaxLength() ? 0 : (getWindowHeight()-280)/14);
            drone.dataReceiver.commandHistory.setMaxLength(drone.controller.commandHistory.getMaxLength());
            break;
            
        //case 'f': ofToggleFullscreen(); break;
    }
    
    keys[key] = true;

    
    
    
    
}

void ardroneApp::loadMovieFile( const fs::path &moviePath )
{
    std::string errorMessage = "";
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie.reset();
		mMovie = ffmpeg::MovieGl::create( moviePath );
		//mMovie->setLoop();
		mMovie->play();
        setInfo(moviePath.filename().string());
        if (mIsEqualMovieSize) getWindow()->setSize(mMovie->getWidth(), mMovie->getHeight());
	}
	catch( exception& e ) {
        errorMessage = string("Unable to load the movie. ") + string(e.what());
        console() << errorMessage << std::endl;
    }
    
    setInfo(moviePath.filename().string(), errorMessage);

	mFrameTexture.reset();
}

void ardroneApp::setInfo(const std::string& path, const std::string& errorMessage){
    // create a texture for showing some info about the movie
    TextLayout infoText;
    infoText.clear( ColorA( 0.2f, 0.2f, 0.2f, 0.25f ) );
    infoText.setColor( Color::white() );
    infoText.addCenteredLine( path);
    if (!errorMessage.empty()) {
        infoText.addLine(errorMessage);
    } else {
        infoText.addLine( toString( mMovie->getWidth() ) + " x " + toString( mMovie->getHeight() ) + " pixels" );
        if (mMovie->getDuration() > 0.f) infoText.addLine( toString( mMovie->getDuration() ) + " seconds" );
        infoText.addLine( toString( mMovie->getNumFrames() ) + " frames" );
        infoText.addLine( toString( mMovie->getFramerate() ) + " fps" );
    }
    infoText.setBorder( 4, 2 );
    mInfoTexture = gl::Texture( infoText.render( true ) );
}

void ardroneApp::fileDrop( FileDropEvent event )
{
	loadMovieFile( event.getFile( 0 ) );
}

void ardroneApp::update()
{
	if( mMovie )
		mFrameTexture = mMovie->getTexture();

    float s = 0.02;
    
    if(keys[cinder::app::KeyEvent::KEY_UP]) drone.controller.pitchAmount -= s;
    else if(keys[cinder::app::KeyEvent::KEY_DOWN]) drone.controller.pitchAmount += s;
    
    if(keys['a']) drone.controller.rollAmount -= s;
    else if(keys['d']) drone.controller.rollAmount += s;
    
    if(keys['w']) drone.controller.liftSpeed += s;
    else if(keys['s']) drone.controller.liftSpeed -= s;
    
    if(keys[cinder::app::KeyEvent::KEY_LEFT]) drone.controller.spinSpeed -= s;
    else if(keys[cinder::app::KeyEvent::KEY_RIGHT]) drone.controller.spinSpeed += s;
    
    // update the drone (process and send queued commands to drone, receive commands from drone and update state
    drone.update();
    
    // update position of simulator (OPTIONAL)
    droneSimulator.update();
    
    TextLayout infoText;
    infoText.clear( ColorA( 0.2f, 0.2f, 0.2f, 0.25f ) );
    infoText.setColor( Color::white() );
//    infoText.addCenteredLine( path);
//    if (!errorMessage.empty()) {
//        infoText.addLine(errorMessage);
//    } else {
//        infoText.addLine( toString( mMovie->getWidth() ) + " x " + toString( mMovie->getHeight() ) + " pixels" );
//        if (mMovie->getDuration() > 0.f) infoText.addLine( toString( mMovie->getDuration() ) + " seconds" );
//        infoText.addLine( toString( mMovie->getNumFrames() ) + " frames" );
//        infoText.addLine( toString( mMovie->getFramerate() ) + " fps" );
//    }
    infoText.setBorder( 4, 2 );


    
    string controllerString = "fps: " + toString(getFrameRate()) + "\n";
    controllerString += "millisSinceLastSend: " + toString(drone.controller.getMillisSinceLastSend()) + "\n";
    controllerString += "\n";
    controllerString += "takeOff (t)\n";
    controllerString += "land (l)\n";
    controllerString += "calibrateHorizontal (c)\n";
    controllerString += "calibrateMagnetometer (m)\n";
    controllerString += "EMERGENCY (E)\n";
    controllerString += "\n";
    controllerString += "roll (a/d)        : " + toString(drone.controller.rollAmount) + "\n";
    controllerString += "pitch (up/down)   : " + toString(drone.controller.pitchAmount) + "\n";
    controllerString += "lift (w/s)        : " + toString(drone.controller.liftSpeed) + "\n";
    controllerString += "spin (left/right) : " + toString(drone.controller.spinSpeed) + "\n";
    controllerString += "\n";
    controllerString += "reset droneSimulator (r)\n";
    controllerString += "debug history (h)\n";
    controllerString += "fullscreen (f)\n";
    controllerString += "PAUSE (p)\n";
    
    infoText.addCenteredLine( controllerString);
    
    ofxARDrone::State &state = drone.state;
    string stateString = "";
    stateString += "isFlying : " + toString(state.isFlying()) + "\n";
    stateString += "isTakingOff : " + toString(state.isTakingOff()) + ", " + toString(state.isTakingOffMillis()) + "\n";
    stateString += "isLanding : " + toString(state.isLanding()) + ", " + toString(state.isLandingMillis()) + "\n";
    stateString += "isCalibratingHorizontal : " + toString(state.isCalibratingHorizontal()) + ", " + toString(state.isCalibratingHorizontalMillis()) + "\n";
    stateString += "isCalibratingMagnetometer : " + toString(state.isCalibratingMagnetometer()) + ", " + toString(state.isCalibratingMagnetometerMillis()) + "\n";
    
    
    stateString += "\n\nisConnected: " + toString(state.isConnected()) + ", " + toString(state.isCalibratingMagnetometerMillis()) + "\n";
    stateString += "altitude: "+ toString(state.getAltitude())+"\n";
    stateString += "emergency state: "+ toString(state.inEmergencyMode())+"\n";
    stateString += "battery level: "+ toString(state.getBatteryPercentage())+"%\n";
    stateString += "vx: "+ toString(state.getVx())+" vy: "+ toString(state.getVy())+" vz: "+ toString(state.getVz())+"\n";
    
    infoText.addCenteredLine(stateString);
    mInfoTexture = gl::Texture( infoText.render( true ) );
}

/** @brief Draws x,y,z axes representing the current reference frame
 *  @detail Axes are drawn in red (+x), green (+y) and blue (+z)
 *	@param size size at which to draw the axes
 **/
void ofDrawAxis(float size) {
    gl::pushMatrices();
    gl::lineWidth(3);

    // draw x axis
    gl::color(Color(1,0,0));
    gl::drawLine(Vec3f(0, 0, 0), Vec3f (size, 0, 0));

    // draw y axis
    gl::color(Color(0,1,0));
    gl::drawLine(Vec3f(0, 0, 0), Vec3f(0, size, 0));

    // draw z axis
    gl::color(Color(0,0,1));
    gl::drawLine(Vec3f(0, 0, 0), Vec3f(0, 0, size));
    gl::popMatrices();

}

void ardroneApp::drawDrone(){
    //gl::clear();
    //gl::enableAlphaBlending();
    glEnable(GL_DEPTH_TEST);
    
    // draw bars
    {
        int x = 155;
        int y = 140;
        int w = 150;
        int h = 14;
        int yinc = 14;
        
        drawBar(x, y, w, h, drone.controller.rollAmount, false); y+= yinc;
        drawBar(x, y, w, h, drone.controller.pitchAmount, false); y+= yinc;
        drawBar(x, y, w, h, drone.controller.liftSpeed, false); y+= yinc;
        drawBar(x, y, w, h, drone.controller.spinSpeed, false); y+= yinc;
    }
    
    // draw simulator
    cout<<"easyCam.begin()";
    cout<<"ofPushStyle()";
    // draw axis
    gl::lineWidth(1);
    ofDrawAxis(100);
    
    // draw thrust line
    gl::color(Color(1, 0, 0));
    gl::lineWidth(3);
    gl::color(Color(0.5, 0.5,0.5));
    gl::drawLine(Vec3f(0, 0, 0), droneSimulator.getPosition());
    
    droneSimulator.draw();
    cout<<"ofPopStyle(); \
    easyCam.end()";
    
    // draw debug strings
//    gl::color(Color(0, 0.8, 0));
//    gl::drawString(controllerString, Vec2f( 10, 30 ));
//    gl::drawString(stateString, Vec2f( getWindowWidth()-300, 30));
    
//    gl::drawString(drone.controller.commandHistory.getAsString(), Vec2f( 10, 280));
//    gl::drawString(drone.dataReceiver.commandHistory.getAsString("\n"), Vec2f( getWindowWidth()-300, 280));
}

void ardroneApp::draw()
{
    renderFbo.bindFramebuffer();

	gl::clear( Color( 0, 0, 0 ) );
    gl::color( Color::white() );
	if ( mFrameTexture ) {
		Rectf centeredRect = Rectf( mFrameTexture.getBounds() ).getCenteredFit( getWindowBounds(), true );
		gl::draw( mFrameTexture, centeredRect  );
	}
    
    renderFbo.blitToScreen(renderFbo.getBounds(), getWindowBounds());
    mSyphonServer.publishTexture(renderFbo.getTexture(), false);

    renderFbo.unbindFramebuffer(); // return rendering to the window's own frame buffer

	if( mInfoTexture ) {
		glDisable( GL_TEXTURE_RECTANGLE_ARB );
		gl::draw( mInfoTexture, Vec2f( 5, getWindowHeight() - 5 - mInfoTexture.getHeight() ) );
	}

    drawDrone();

}

 //check for division by zero???
 //--------------------------------------------------
 float ardroneApp::ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
     
     if (fabs(inputMin - inputMax) < FLT_EPSILON){
         cerr<<("ofMath") << "ofMap(): avoiding possible divide by zero, check inputMin and inputMax: " << inputMin << " " << inputMax<<endl;
         return outputMin;
     } else {
         float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
         
         if( clamp ){
             if(outputMax < outputMin){
                 if( outVal < outputMax )outVal = outputMax;
                 else if( outVal > outputMin )outVal = outputMin;
             }else{
                 if( outVal > outputMax )outVal = outputMax;
                 else if( outVal < outputMin )outVal = outputMin;
             }
         }
         return outVal;
     }
     
 }
void ardroneApp::drawBar(int x, int y, int w, int h, float t, bool doVertical) {
    cout<<"ofPushStyle()";

    gl::color(Color(1, 0, 0));
    if(doVertical) gl::drawSolidRect(Rectf(x, y + h/2, w, ofMap(t, -1, 1, h/2, -h/2, false)));
    else gl::drawSolidRect(Rectf(x + w/2, y, ofMap(t, -1, 1, -w/2, w/2, false), h));

    gl::color(Color(0.5, 0.5, 0.5));
    gl::lineWidth(1);
    gl::drawStrokedRect(Rectf(x,y,w,h));
    if(doVertical) gl::drawLine(Vec2f(x, y - y/2), Vec2f(x + w, y + y/2));
    else gl::drawLine(Vec2f(x + w/2, y), Vec2f(x + w/2, y + h));
    cout<<"ofPopStyle()";
}

CINDER_APP_NATIVE( ardroneApp, RendererGl )
