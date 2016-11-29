#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Ubo.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include "cinder/gl/Texture.h"
#include "cinder/MayaCamUI.h"
#include "Resources.h"


#define INPUT_DEVICE "Scarlett 2i2 USB"

using namespace std;
using namespace ci;
using namespace ci::app;

class VideoAudioSuperformulaApp : public AppNative {
  public:
	void	setup() override;
	void	resize() override;
	void	update() override;
	void	draw() override;
    void    mouseWheel( MouseEvent event ) override;
    void    mouseDown( MouseEvent event) override;
    void    mouseDrag( MouseEvent) override;
    void    keyDown(KeyEvent) override;

	void	setupGeometry();
	
	CameraPersp				mCam;
	gl::BatchRef			mBatch, mNormalsBatch;
	gl::GlslProgRef			mGlsl, mNormalsGlsl;
	mat4					mRotation;
#if ! defined( CINDER_GL_ES )
	params::InterfaceGlRef	mParams;
#endif

	bool					mDrawNormals;
	float					mNormalsLength;
	int						mSubdivisions;
	int						mCheckerFrequency;
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    std::vector<float>					mMagSpectrum;
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    gl::TextureRef			mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;
    gl::TextureRef			mTexture;
    uint32_t mOffset;
    MayaCamUI			mMayaCam;
    float mFrameRate;
	
	// This is dependent on the C++ compiler structuring these vars in RAM the same way that GL's std140 does
	struct {
		float	mA1, mA2;
		float	mB1, mB2;
		float	mM1, mM2;
		float	mN11, mN12;
		float	mN21, mN22;
		float	mN31, mN32;
	} mFormulaParams;
	gl::UboRef				mFormulaParamsUbo;
};

void VideoAudioSuperformulaApp::keyDown(cinder::app::KeyEvent event){
    switch (event.getChar()) {
        case KeyEvent::KEY_f:
            setFullScreen(!isFullScreen());
            break;
            
        default:
            break;
    }
}

void VideoAudioSuperformulaApp::setupGeometry()
{
	auto plane = geom::Plane().subdivisions( ivec2( mSubdivisions, mSubdivisions ) );
	mBatch = gl::Batch::create( plane, mGlsl );
	mNormalsBatch = gl::Batch::create( geom::VertexNormalLines( plane, 0.0f ), mNormalsGlsl );
}

void VideoAudioSuperformulaApp::setup()
{
	mFormulaParams.mA1 = mFormulaParams.mA2 = 1.0f;
	mFormulaParams.mB1 = mFormulaParams.mB2 = 1.0f;
	mFormulaParams.mN11 = mFormulaParams.mN12 = 1.0f;
	mFormulaParams.mN21 = mFormulaParams.mN22 = 1.0f;
	mFormulaParams.mM1 = 4.0f;
	mFormulaParams.mM2 = 6.0f;
	mFormulaParams.mN31 = 2.0f;
	mFormulaParams.mN32 = 2.5f;

	mDrawNormals = false;
	mNormalsLength = 0.20f;
	mSubdivisions = 100;
	mCheckerFrequency = 7;

#if ! defined( CINDER_GL_ES )
	mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );
    mParams->addParam( "Frame rate",	&mFrameRate,"", true);
	mParams->addParam( "A (1)", &mFormulaParams.mA1 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "B (1)", &mFormulaParams.mB1 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "M (1)", &mFormulaParams.mM1 ).min( 0 ).max( 20 ).step( 0.25f );
	mParams->addParam( "N1 (1)", &mFormulaParams.mN11 ).min( 0 ).max( 100 ).step( 1.0 );
	mParams->addParam( "N2 (1)", &mFormulaParams.mN21 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addParam( "N3 (1)", &mFormulaParams.mN31 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addSeparator();
	mParams->addParam( "A (2)", &mFormulaParams.mA2 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "B (2)", &mFormulaParams.mB2 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "M (2)", &mFormulaParams.mM2 ).min( 0 ).max( 20 ).step( 0.25f );
	mParams->addParam( "N1 (2)", &mFormulaParams.mN12 ).min( 0 ).max( 100 ).step( 1.0 );
	mParams->addParam( "N2 (2)", &mFormulaParams.mN22 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addParam( "N3 (2)", &mFormulaParams.mN32 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addSeparator();
	mParams->addParam( "Subdivisions", &mSubdivisions ).min( 5 ).max( 500 ).step( 30 ).updateFn( [&] { setupGeometry(); } );
	mParams->addParam( "Checkerboard", &mCheckerFrequency ).min( 1 ).max( 500 ).step( 3 );
	mParams->addSeparator();
	mParams->addParam( "Draw Normals", &mDrawNormals );
	mParams->addParam( "Normals Length", &mNormalsLength ).min( 0.0f ).max( 2.0f ).step( 0.025f );
#endif

	mCam.lookAt( vec3( 3, 2, 4 ) * 0.75f, vec3( 0 ) );

#if defined( CINDER_GL_ES_3 )
	mGlsl = gl::GlslProg::create( loadAsset( "shader_es3.vert" ), loadAsset( "shader_es3.frag" ) );
	mNormalsGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "normals_shader_es3.vert" ) )
											.fragment( loadAsset( "normals_shader_es3.frag" ) )
											.attrib( geom::CUSTOM_0, "vNormalWeight" ) );
#else
	mGlsl = gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );
	mNormalsGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "normals_shader.vert" ) )
											.fragment( loadAsset( "normals_shader.frag" ) )
											.attrib( geom::CUSTOM_0, "vNormalWeight" ) );
#endif

	// allocate our UBO
	mFormulaParamsUbo = gl::Ubo::create( sizeof( mFormulaParams ), &mFormulaParams, GL_DYNAMIC_DRAW );
	// and bind it to buffer base 0; this is analogous to binding it to texture unit 0
	mFormulaParamsUbo->bindBufferBase( 0 );
	// and finally tell the shaders that their uniform buffer 'FormulaParams' can be found at buffer base 0
	mGlsl->uniformBlock( "FormulaParams", 0 );
	mNormalsGlsl->uniformBlock( "FormulaParams", 0 );

	setupGeometry();

	gl::enableDepthWrite();
	gl::enableDepthRead();
    
    auto ctx = audio::Context::master();
    std::vector<audio::DeviceRef> devices = audio::Device::getInputDevices();
    const auto dev = audio::Device::findDeviceByName(INPUT_DEVICE);
    if (!dev){
        cout<<"Could not find " << INPUT_DEVICE << endl;
        mInputDeviceNode = ctx->createInputDeviceNode();
        cout<<"Using default input"<<endl;
    } else {
        mInputDeviceNode = ctx->createInputDeviceNode(dev);
    }
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    getWindow()->setTitle( mInputDeviceNode->getDevice()->getName() );
    
    // create channels from which we can construct our textures
    mChannelLeft = Channel32f(kBands, kHistory);
    mChannelRight = Channel32f(kBands, kHistory);
    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );
    
    // create texture format (wrap the y-axis, clamp the x-axis)
//    mTextureFormat.setWrapS( GL_CLAMP );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );

    mTexture = gl::Texture::create( loadImage( loadResource( RES_LANDSCAPE_IMAGE) ) );

}

void VideoAudioSuperformulaApp::resize()
{
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	gl::setMatrices( mCam );
}

void VideoAudioSuperformulaApp::mouseWheel( MouseEvent event )
{
    // Zoom in/out with mouse wheel
    vec3    eye = mCam.getEyePoint();
    eye.z += event.getWheelIncrement();
    mCam.setEyePoint( eye );
}

void VideoAudioSuperformulaApp::mouseDown( MouseEvent event )
{
    
    mMayaCam.setCurrentCam(mCam);
    mMayaCam.mouseDown( event.getPos() );
    //cout<<mMayaCam.getCamera().getEyePoint()<<endl;
}

void VideoAudioSuperformulaApp::mouseDrag( MouseEvent event )
{
    // handle mouse drag
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    mCam = mMayaCam.getCamera();
    //cout<<"D "<<mMayaCam.getCamera().getEyePoint()<<endl;
}

void VideoAudioSuperformulaApp::update()
{
	// Rotate the by 2 degrees around an arbitrary axis
	vec3 axis = vec3( cos( getElapsedSeconds() / 3 ), sin( getElapsedSeconds() / 2 ), sin( getElapsedSeconds() / 5 ) );
	mRotation *= rotate( toRadians( 0.2f ), normalize( axis ) );

	// buffer our data to our UBO to reflect any changed parameters
	mFormulaParamsUbo->bufferSubData( 0, sizeof( mFormulaParams ), &mFormulaParams );
	
	mNormalsBatch->getGlslProg()->uniform( "uNormalsLength", mNormalsLength );
	mBatch->getGlslProg()->uniform( "uCheckerFrequency", mCheckerFrequency );
    
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    mOffset = (mOffset+1) % kHistory;
    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);
    
    float offSt = mOffset / float(kHistory);
    mBatch->getGlslProg()->uniform("uTexOffset", offSt);
    mBatch->getGlslProg()->uniform("uVideoTex", 0);
    mBatch->getGlslProg()->uniform("uLeftTex", 1);
    mBatch->getGlslProg()->uniform("uRightTex", 2);

    mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
    mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
    
    mFrameRate = getAverageFps();

}

void VideoAudioSuperformulaApp::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );

	gl::setMatrices( mCam );
	gl::pushMatrices();
		gl::multModelMatrix( mRotation );
    
        mTexture->bind(0);
        mTextureLeft->bind(1);
        mTextureRight->bind(2);

		mBatch->draw();
		gl::color( 1.0f, 1.0f, 1.0f, 1 );
		if( mDrawNormals )
			mNormalsBatch->draw();
    
        mTextureLeft->unbind();
        mTextureRight->unbind();
    mTexture -> unbind();
	gl::popMatrices();

#if ! defined( CINDER_GL_ES )
	mParams->draw();
#endif
}

CINDER_APP_NATIVE( VideoAudioSuperformulaApp, RendererGl )