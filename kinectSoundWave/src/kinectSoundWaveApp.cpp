#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "Kinect.h"
#include "Resources.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"

static const int VBO_X_RES  = 640;
static const int VBO_Y_RES  = 480;

using namespace ci;
using namespace ci::app;
using namespace std;

class kinectPointCloudApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void createVbo();
    void mouseDrag(MouseEvent e);
	void update();
	void draw();
	
	// PARAMS
	params::InterfaceGl	mParams;
	
	// CAMERA
	CameraPersp		mCam;
	Quatf			mSceneRotation;
	Vec3f			mEye, mCenter, mUp;
	float			mCameraDistance;
	float			mKinectTilt;
	
	// KINECT AND TEXTURES
	Kinect			mKinect;
	gl::Texture		mDepthTexture;
	float			mScale;
	float			mXOff, mYOff;
	
	// VBO AND SHADER
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mShader;
    float mouseY;
    
    uint16_t bandCount;
    audio::Input mInput;
    audio::PcmBuffer32fRef mPcmBuffer;
    std::shared_ptr<float> mFftDataRef;
    
    void updateWave();
    void setupAudio();
    Surface32f	    audioSurface;
    gl::Texture		audioTexture;
    
    Font mFont;
    
    float waveWidth, waveStep, waveAmplitude;
};

void kinectPointCloudApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void kinectPointCloudApp::updateWave()
{	
	if( ! mPcmBuffer ) {
		return;
	}
	
	uint32_t bufferLength = mPcmBuffer->getSampleCount();
	audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
	audio::Buffer32fRef rightBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_RIGHT );
    
	int displaySize = getWindowWidth();
	float scale = displaySize / (float)bufferLength;
	
	PolyLine<Vec2f>	leftBufferLine;
	PolyLine<Vec2f>	rightBufferLine;
	
	for( int i = 0; i < bufferLength; i++ ) {
		float x = ( i * scale );
        
		//get the PCM value from the left channel buffer
		float y = ( ( leftBuffer->mData[i] - 1 ) * - 100 );
		leftBufferLine.push_back( Vec2f( x , y) );
		
		y = ( ( rightBuffer->mData[i] - 1 ) * - 100 );
		rightBufferLine.push_back( Vec2f( x , y) );
	}
//	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
//	gl::draw( leftBufferLine );
//	gl::draw( rightBufferLine );

    
    Surface32f::Iter pixelIter = audioSurface.getIter();
    int i = 0;
    int ln = 0;
	while( pixelIter.line() ) {
        i = 0;
		while( pixelIter.pixel() ) {
		    float y = (ln % (int)waveStep == 0)? ( ( leftBuffer->mData[i] - 1 ) * - 100 ): 0.0;//;
			audioSurface.setPixel( pixelIter.getPos(), ColorAf(y, 0.0f, 0.0f, 1.0f ) );
            i++;
		}
        ln ++;
	}
    
    audioTexture.update(audioSurface);
}


void kinectPointCloudApp::setupAudio() {
	//iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
	mInput = audio::Input();
    
	//tell the input to start capturing audio
	mInput.start();
}

void kinectPointCloudApp::setup()
{	
    waveWidth = 5.0;
    waveStep = 10.0;
    waveAmplitude = 1.0;
	// SETUP PARAMS
	mParams = params::InterfaceGl( "KinectSoundCloud", Vec2i( 200, 180 ) );
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams.addParam( "Cam Distance", &mCameraDistance, "min=100.0 max=5000.0 step=100.0 keyIncr=D keyDecr=d" );
	mParams.addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
    mParams.addSeparator();
    mParams.addParam( "Wave Width", &waveWidth, "min=1 max=20 keyIncr=W keyDecr=w");
    mParams.addParam( "Wave Step", &waveStep, "min=1 max=100 keyIncr=S keyDecr=s");
    mParams.addParam("Wave Amplitude", &waveAmplitude, "min=0 max=600 keyIncr=A keyDecr=a");

	
	// SETUP CAMERA
	mCameraDistance = 1000.0f;
	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
	
	// SETUP KINECT AND TEXTURES
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
	mKinect			= Kinect( Kinect::Device() ); // use the default Kinect
	mDepthTexture	= gl::Texture( 640, 480 );
	
	// SETUP VBO AND SHADER	
	createVbo();
	mShader	= gl::GlslProg( loadResource( RES_VERT_ID ), loadResource( RES_FRAG_ID ) );
	
	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();
    
    mouseY = 0.0;
    setupAudio();
    
    mFont = Font( "Times New Roman", 22.0f );
}

void kinectPointCloudApp::createVbo()
{
	gl::VboMesh::Layout layout;
	
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticIndices();
	
	std::vector<Vec3f> positions;
	std::vector<Vec2f> texCoords;
	std::vector<uint32_t> indices; 
	
	int numVertices = VBO_X_RES * VBO_Y_RES;
	int numShapes	= ( VBO_X_RES - 1 ) * ( VBO_Y_RES - 1 );

	mVboMesh		= gl::VboMesh( numVertices, numShapes, layout, GL_POINTS );
	
	for( int x=0; x<VBO_X_RES; ++x ){
		for( int y=0; y<VBO_Y_RES; ++y ){
			indices.push_back( x * VBO_Y_RES + y );

			float xPer	= x / (float)(VBO_X_RES-1);
			float yPer	= y / (float)(VBO_Y_RES-1);
			positions.push_back( Vec3f( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES, 0.0f ) );
			texCoords.push_back( Vec2f( xPer, yPer ) );			
		}
	}
	
	mVboMesh.bufferPositions( positions );
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d(0, texCoords );

    //Velocity 2D texture array
    int SIDE = 1024;
	audioSurface = Surface32f( VBO_X_RES, VBO_Y_RES, true);
    
    gl::Texture::Format tFormat;
	tFormat.setInternalFormat(GL_RGBA32F_ARB);
	audioTexture = gl::Texture( audioSurface, tFormat);
	audioTexture.setWrap( GL_REPEAT, GL_REPEAT );
	audioTexture.setMinFilter( GL_NEAREST );
	audioTexture.setMagFilter( GL_NEAREST );
}

void kinectPointCloudApp::mouseDrag(MouseEvent event)
{
    mouseY = event.getY();
}

void kinectPointCloudApp::update()
{
	if( mKinect.checkNewDepthFrame() )
		mDepthTexture = mKinect.getDepthImage();
	
	// This sample does not use the color data
	//if( mKinect.checkNewVideoFrame() )
	//	mColorTexture = mKinect.getVideoImage();

	if( mKinectTilt != mKinect.getTilt() )
		mKinect.setTilt( mKinectTilt );
		
	mEye = Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
    
    mPcmBuffer = mInput.getPcmBuffer();
    if( ! mPcmBuffer ) {
        return;
    }
    
    bandCount = 125;
    mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
    
    float ht = 1000.0f;

    //mouseY = mFftDataRef.get()[0] / bandCount * ht;
    
    
    // dynmaically generate our new positions based on a simple sine wave for mesh
//    gl::VboMesh::VertexIter iter2 = mVboMesh.mapVertexBuffer();
//    
//    for( int x=0; x<VBO_X_RES; ++x ){
//		for( int y=0; y<VBO_Y_RES; ++y ){
//            
//			float xPer	= x / (float)(VBO_X_RES-1);
//			float yPer	= y / (float)(VBO_Y_RES-1);
//			
//            iter2.setPosition( Vec3f( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES, 0.0f ) );
//            
//		}
//	}
    updateWave();

}

void kinectPointCloudApp::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ), true );
	
	gl::pushMatrices();
    glPointSize(waveWidth);
		gl::scale( Vec3f( -1.0f, -1.0f, 1.0f ) );
		gl::rotate( mSceneRotation );
		mDepthTexture.bind( 0 );
        audioTexture.bind(1);
		mShader.bind();
		mShader.uniform("depthTex", 0 );
        mShader.uniform("soundTex", 1 );
        mShader.uniform("waveAmplitude", waveAmplitude);
		gl::draw( mVboMesh );
		mShader.unbind();
	gl::popMatrices();
    
	params::InterfaceGl::draw();
    
    gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(132.0f, 52.0f),Color::white(),mFont);

}


CINDER_APP_BASIC( kinectPointCloudApp, RendererGl )
