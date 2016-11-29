#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

#include "Resources.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
using namespace ci;
using namespace ci::app;

class HelloWorldShaderToyApp : public AppBasic {
public:
	void setup();
	
	void update();
	void draw();
	
    gl::Texture mFrameTexture;
	gl::GlslProgRef	mShader;
    
    void loadMovieFile( const fs::path &path );
    void fileDrop( FileDropEvent event );

	qtime::MovieGlRef		mMovie;
};



void HelloWorldShaderToyApp::setup()
{
	
	try {
		mShader = gl::GlslProg::create( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
    
}

void HelloWorldShaderToyApp::fileDrop( FileDropEvent event )
{
	loadMovieFile( event.getFile( 0 ) );
}

void HelloWorldShaderToyApp::loadMovieFile( const fs::path &moviePath )
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create( moviePath );
		mMovie->setLoop();
		mMovie->play();
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie->reset();
	}
    
	mFrameTexture.reset();
}

void HelloWorldShaderToyApp::update()
{
    if( mMovie )
        mFrameTexture = mMovie->getTexture();
    
}

void HelloWorldShaderToyApp::draw()
{
    if (!mFrameTexture) return;
	gl::clear();
    
    if (mFrameTexture) {
        mFrameTexture.enableAndBind();
    }
	mShader->bind();
	mShader->uniform( "iChannel0", 0 );
    mShader->uniform( "iResolution", Vec3f( mFrameTexture.getWidth(), mFrameTexture.getHeight(), 0.0f ) );
    
	gl::drawSolidRect( getWindowBounds() );
    
    if (mFrameTexture) {
        mFrameTexture.unbind();
    }
}

CINDER_APP_BASIC( HelloWorldShaderToyApp, RendererGl )