#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "Resources.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/params/Params.h"
using namespace ci;
using namespace ci::app;

class BritneysSpaceshipApp : public AppBasic {
public:
	void setup();
	void keyDown( KeyEvent event );
	
	void update();
	void draw();
	
	gl::TextureRef	soundTexture;
    gl::Texture mFrameTexture;
	gl::GlslProgRef	mShader;
    
    audio::Input mInput;
    std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
    
    void loadMovieFile( const fs::path &path );
    void fileDrop( FileDropEvent event );
    void mouseDrag( MouseEvent event );
    Vec3f mouse;

	qtime::MovieGlRef		mMovie;
    cinder::params::InterfaceGlRef                  mParams;
    float mFrameRate;
};



void BritneysSpaceshipApp::setup()
{
    
    mParams = params::InterfaceGl::create( "Params", Vec2i( 220, 300 ) );
    mParams->addParam( "Frame rate",		&mFrameRate,				"", true );

	
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
    
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	mInput = audio::Input();
    
	mInput.start();
    
}

void BritneysSpaceshipApp::fileDrop( FileDropEvent event )
{
	loadMovieFile( event.getFile( 0 ) );
}

void BritneysSpaceshipApp::loadMovieFile( const fs::path &moviePath )
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

void BritneysSpaceshipApp::mouseDrag( MouseEvent event )
{
	mouse = Vec3f(event.getX(), event.getY(), 10 );
}

void BritneysSpaceshipApp::keyDown( KeyEvent event )
{
	if( event.getCode() == app::KeyEvent::KEY_f ) {
		setFullScreen( ! isFullScreen() );
	}
}

void BritneysSpaceshipApp::update()
{
    mFrameRate	= getAverageFps();
        getWindow()->setTitle(toString(mFrameRate));
    return;
    mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}
    
    /*
     shadertoy: The FFT signal, which is 512 pixels/frequencies long, gets normalized to 0..1 and mapped to 0..255.
     The wave form, which is also 512 pixels/sampled long, gets renormalized too from -16387..16384 to 0..1. (and then to 0..255???)
     FFT goes in the first row, waveform in the second row. So this is a 512x2 gray scale 8 bit texture.
     */
    
	uint16_t bandCount = 512;
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
    
    float * fftBuffer = mFftDataRef.get();
    if (!fftBuffer) return;
    
	
    //create a sound texture as 512x2
	unsigned char signal[1024];
    
    //the first row is the spectrum (shadertoy)
    float max = 0.;
    for(int i=0;i<512;++i){
		if (fftBuffer[i] > max) max = fftBuffer[i];
    }
    
    float ht = 255.0/max;
	for(int i=0;i<512;++i){
		signal[i] = (unsigned char) (fftBuffer[i] * ht);
    }
    
    //waveform
    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
	audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
    
	int endIdx = bufferSamples;
	
	//only use the last 1024 samples or less
	int32_t startIdx = ( endIdx - 1024 );
	startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float mx = -FLT_MAX,mn = FLT_MAX, val;
	for( uint32_t i = startIdx; i < endIdx; i++) {
        val = leftBuffer->mData[i];
        if (val > mx) mx = val;
        if (val < mn) mn = val;
	}
    
    float scale = 1./(mx - mn);
    for( uint32_t i = startIdx, c = 512; c < 1024; i++, c++ ) {
        signal[c] = (unsigned char) ((leftBuffer->mData[i]-mn)*scale);
	}
    
	// store it as a 512x2 texture

	soundTexture = std::make_shared<gl::Texture>( signal, GL_LUMINANCE, 512, 2 );
    
    if( mMovie )
    mFrameTexture = mMovie->getTexture();

}

void BritneysSpaceshipApp::draw()
{
	gl::clear();

    if (mFrameTexture) mFrameTexture.enableAndBind();
    
	mShader->bind();
	mShader->uniform( "iChannel0", 0 );
    mShader->uniform( "iResolution", Vec3f( getWindowWidth(), getWindowHeight(), 0.0f ) );
    mShader->uniform( "iGlobalTime", float( getElapsedSeconds() ) );
    mShader->uniform("iMouse", mouse);
    
	gl::drawSolidRect( getWindowBounds() );
    
    
    if (mFrameTexture) mFrameTexture.unbind();
    mParams->draw();

}


CINDER_APP_BASIC( BritneysSpaceshipApp, RendererGl )