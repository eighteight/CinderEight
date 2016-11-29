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
	if( event.getChar() == 'o' ) {
		fs::path moviePath = getOpenFilePath();
		if( ! moviePath.empty() )
			loadMovieFile( moviePath );
    } else 	if( event.getChar() == 'r' ) {
        setupMovie();
    }
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
}

CINDER_APP_NATIVE( ardroneApp, RendererGl )
