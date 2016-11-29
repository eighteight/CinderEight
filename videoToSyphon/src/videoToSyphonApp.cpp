#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinderSyphon.h"
#include "cinder/qtime/QuickTime.h"
#include <list>

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 800
#define HEIGHT 600

// We'll create a new Cinder Application by deriving from the AppBasic class
class syphonImpApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event );
	void mouseUp( MouseEvent event );
	void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void setup();
	void update();
	void draw();
	
	gl::Texture mTex;
	float mRot;
	uint32_t    mWidth, mHeight;
	syphonServer mScreenSyphon;
	//syphonServer mTextureSyphon;
	
	//syphonClient mClientSyphon;
	
private:
	void setMoviePlayer();
	void loadMovieFile( const string &moviePath );
	qtime::MovieGl		mMovie;
};

void syphonImpApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(WIDTH,HEIGHT);
	settings->setFrameRate(60.f);
}

void syphonImpApp::setup()
{
	setMoviePlayer();
	mScreenSyphon.setName("Cinder Screen");
	//mTextureSyphon.setName("Cinder Texture");
	
//	mClientSyphon.setup();
    
//    mClientSyphon.setApplicationName("Simple Server");
//    mClientSyphon.setServerName("");
}

void syphonImpApp::update()
{
		mTex = mMovie.getTexture();
}

void syphonImpApp::draw()
{
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
	
	gl::pushModelView();
	gl::translate(Vec2f(getWindowWidth()/2, getWindowHeight()/2));
	gl::rotate(mRot);
	gl::scale(Vec3f(4.f, 4.f, 1.f));
	gl::color(ColorA(1.f, 0.f, 0.f, 1.f));
	gl::popModelView();
	
	if(mTex){
		gl::color(ColorA(1.f, 1.f, 1.f, 1.f));
		gl::draw(mTex);
	}
	
	mScreenSyphon.publishScreen();
	//mTextureSyphon.publishTexture(&mTex);
	
	//mClientSyphon.draw(0, 0);
}

void syphonImpApp::keyDown( KeyEvent event )
{
	//
}

void syphonImpApp::mouseDown( MouseEvent event )
{
	//
}

void syphonImpApp::mouseUp( MouseEvent event )
{
	//
}

void syphonImpApp::mouseMove( MouseEvent event )
{
	//
}

void syphonImpApp::mouseDrag( MouseEvent event )
{
	//
}

void syphonImpApp::setMoviePlayer() {
	string moviePath = "/Users/eight/Desktop/water.mov";//getOpenFilePath();
	console() << "Path: " << moviePath << std::endl;
	if( ! moviePath.empty() )
		loadMovieFile( moviePath );
}

void syphonImpApp::loadMovieFile( const string &moviePath )
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl( moviePath );
		mWidth = mMovie.getWidth();
		mHeight = mMovie.getHeight();
		mMovie.setLoop();
		mMovie.play();
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();
	}
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( syphonImpApp, RendererGl )

