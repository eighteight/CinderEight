#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

#include "cinder/audio/Context.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Utilities.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class AudioShaderApp : public AppNative {
public:
	void setup();
	void update();
	void draw();
    
private:
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
};

void AudioShaderApp::setup()
{
	// load and compile shader
	try {
		mShader = gl::GlslProg( loadResource("audio_surf_vert.glsl"), loadResource("audio_surf_frag.glsl") );
	}
	catch( const std::exception& e )
	{
		console() << e.what() << std::endl;
	}
    
    
    auto ctx = audio::Context::master();
    
    // The InputDeviceNode is platform-specific, so you create it using a special method on the Context:
    mInputDeviceNode = ctx->createInputDeviceNode();
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( 2048 ).windowSize( 1024 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
}

void AudioShaderApp::update()
{
    
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    size_t numBins = mMonitorSpectralNode->getFftSize() / 2;
//    size_t bin = min( numBins - 1, size_t( ( numBins * ( mouseX - mSpectrumPlot.getBounds().x1 ) ) / mSpectrumPlot.getBounds().getWidth() ) );
//    
//    float binFreqWidth = mMonitorSpectralNode->getFreqForBin( 1 ) - mMonitorSpectralNode->getFreqForBin( 0 );
//    float freq = mMonitorSpectralNode->getFreqForBin( bin );
//    float mag = audio::linearToDecibel( mMagSpectrum[bin] );
//    
//    mPcmBuffer = mInput.getPcmBuffer();
//	if( ! mPcmBuffer ) {
//		return;
//	}
//    
//    /*
//     shadertoy: The FFT signal, which is 512 pixels/frequencies long, gets normalized to 0..1 and mapped to 0..255.
//     The wave form, which is also 512 pixels/sampled long, gets renormalized too from -16387..16384 to 0..1. (and then to 0..255???)
//     FFT goes in the first row, waveform in the second row. So this is a 512x2 gray scale 8 bit texture.
//     */
//    
//	uint16_t bandCount = 512;
//	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
//    
//    float * fftBuffer = mFftDataRef.get();
//    if (!fftBuffer) return;
//    
	
    //create a sound texture as 512x2
	unsigned char signal[1024];
    
    //the first row is the spectrum (shadertoy)
    float max = 0.;
    for(int i=0;i<512;++i){
        if (mMagSpectrum[i] > max) max = mMagSpectrum[i];
    }
    
    float ht = 255.0/max;
    
    for(int i = 0; i < numBins; i++) {
       signal[i] = (unsigned char) (audio::linearToDecibel(mMagSpectrum[i]) * audio::linearToDecibel(ht));
    }
    
    //waveform
//    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
//	audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
//    
//	int endIdx = bufferSamples;
//	
//	//only use the last 1024 samples or less
//	int32_t startIdx = ( endIdx - 1024 );
//	startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
//    
//    float mx = -FLT_MAX,mn = FLT_MAX, val;
//	for( uint32_t i = startIdx; i < endIdx; i++) {
//        val = leftBuffer->mData[i];
//        if (val > mx) mx = val;
//        if (val < mn) mn = val;
//	}
//    
//    float scale = 255.0/(mx - mn);
//    for( uint32_t i = startIdx, c = 512; c < 1024; i++, c++ ) {
//        signal[c] = (unsigned char) ((leftBuffer->mData[i]-mn)*scale);
//	}

    
	// store it as a 512x2 texture
	mTexture = gl::Texture( signal, GL_LUMINANCE, 512, 2 );
}

void AudioShaderApp::draw()
{
    if (!mTexture) return;
	gl::clear();
    
	// bind texture to slot 0
	mTexture.enableAndBind();
    
	// bind shader and set uniforms
	mShader.bind();
	mShader.uniform( "iResolution", Vec3f( getWindowWidth(), getWindowHeight(), 0.0f ) );
	mShader.uniform( "iGlobalTime", float( getElapsedSeconds() ) );
	mShader.uniform( "iChannel0", 0 );
    
	// run full screen pass
	gl::drawSolidRect( getWindowBounds() );
    
	// unbind shader and texture
	mShader.unbind();
	mTexture.unbind();
}

CINDER_APP_BASIC( AudioShaderApp, RendererGl );
