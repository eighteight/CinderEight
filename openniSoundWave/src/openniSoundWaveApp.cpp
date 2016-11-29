#define BOOST_HAS_TR1_RANDOM 1
#include "cinder/app/AppBasic.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "Resources.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "VOpenNIHeaders.h"
#include "KinectTextures.h"
#include "cinderSyphon.h"

static const int VBO_X_RES  = 640;
static const int VBO_Y_RES  = 480;

static const int KINECT_COLOR_WIDTH = 640;	//1280;
static const int KINECT_COLOR_HEIGHT = 480;	//1024;
static const int KINECT_COLOR_FPS = 30;	//15;
static const int KINECT_DEPTH_WIDTH = 640;
static const int KINECT_DEPTH_HEIGHT = 480;
static const int KINECT_DEPTH_SIZE = KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH;
static const int KINECT_DEPTH_FPS = 30;

using namespace ci;
using namespace ci::app;
using namespace std;

class openniSoundWave : public AppBasic, V::UserListener {
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
    
private:
    V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;
    uint16_t*				pixels;
    gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
    std::map<int, gl::Texture> mUsersTexMap;
    bool    showUser;
    
    void onNewUser( V::UserEvent event );
	void onLostUser( V::UserEvent event );
    
	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}
    
	ImageSourceRef getUserImage( int id )
	{
		_device0->getLabelMap( id, pixels );
		return ImageSourceRef( new ImageSourceKinectDepth( pixels, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
    //SYPHON
    syphonServer mScreenSyphon; //each item to publish requires a different server
	syphonServer mTextureSyphon;
	
	syphonClient mClientSyphon; //our syphon client
};

void openniSoundWave::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void openniSoundWave::updateWave()
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


void openniSoundWave::setupAudio() {
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

void openniSoundWave::setup()
{	
    waveWidth = 5.0;
    waveStep = 10.0;
    waveAmplitude = 1.0;
	// SETUP PARAMS
	mParams = params::InterfaceGl( "KinectSoundCloud", Vec2i( 200, 180 ) );
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams.addParam( "Cam Distance", &mCameraDistance, "min=100.0 max=5000.0 step=100.0 keyIncr=D keyDecr=d" );
    mParams.addSeparator();
    mParams.addParam( "Wave Width", &waveWidth, "min=1 max=20 keyIncr=W keyDecr=w");
    mParams.addParam( "Wave Step", &waveStep, "min=1 max=100 keyIncr=S keyDecr=s");
    mParams.addParam("Wave Amplitude", &waveAmplitude, "min=0 max=600 keyIncr=A keyDecr=a");
    mParams.addParam ("User?", &showUser, "");

	
	// SETUP CAMERA
	mCameraDistance = 1000.0f;
	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );
	
    // SETUP OPENNI
    
    V::OpenNIDeviceManager::USE_THREAD = false;
	_manager = V::OpenNIDeviceManager::InstancePtr();
    
    //    if( getArgs().size() > 1 )
    //	{
    //        //		nRetVal = m_Context.Init();
    //        //		CHECK_RC( nRetVal, "Init" );
    //        //		nRetVal = m_Context.OpenFileRecording( getArgs()[1] );
    //        //		if( nRetVal != XN_STATUS_OK ){
    //        //			printf( "Can't open recording %s: %s\n", argv[1], xnGetStatusString( nRetVal ) );
    //        //			return 1;
    //        //		}
    
    
    const XnChar* filename = "/Volumes/Data/donttakenew/donttakeoni/20120530-175701.oni";//20120505-160636.oni";
    
    _manager->createDevice(filename/*getArgs()[1]*/, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH | V::NODE_TYPE_USER | V::NODE_TYPE_SCENE );
    //	} else 
    {
        //_manager->createDevices( 1, V::NODE_TYPE_IMAGE | V::NODE_TYPE_DEPTH | V::NODE_TYPE_SCENE | V::NODE_TYPE_USER );
    }
	_device0 = _manager->getDevice( 0 );
    _device0->setDepthShiftMul( 3 );
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Can't find a kinect device\n" );
        quit();
        shutdown();
	}
    _device0->addListener( this );
	pixels = new uint16_t[ KINECT_DEPTH_SIZE ];
	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT );
    
	_manager->start();
    
	
	// SETUP VBO AND SHADER	
	createVbo();
	mShader	= gl::GlslProg( loadResource( RES_VERT_ID ), loadResource( RES_FRAG_ID ) );
	
	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();

    setupAudio();
    
    mFont = Font( "Times New Roman", 22.0f );
    
    //SYPHON
    mScreenSyphon.setName("Cinder Screen"); // set a name for each item to be published
	mTextureSyphon.setName("Cinder Texture");
	
	mClientSyphon.setup();
    
	// in order for this to work, you must run simple server from the testapps directory
	// any other syphon item you create would work as well, just change the name
    mClientSyphon.setApplicationName("Simple Server");
    mClientSyphon.setServerName("");
	
	mClientSyphon.bind();
}

void openniSoundWave::createVbo()
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
	audioSurface = Surface32f( VBO_X_RES, VBO_Y_RES, true);
    
    gl::Texture::Format tFormat;
	tFormat.setInternalFormat(GL_RGBA32F_ARB);
	audioTexture = gl::Texture( audioSurface, tFormat);
	audioTexture.setWrap( GL_REPEAT, GL_REPEAT );
	audioTexture.setMinFilter( GL_NEAREST );
	audioTexture.setMagFilter( GL_NEAREST );
}

void openniSoundWave::mouseDrag(MouseEvent event)
{
    mouseY = event.getY();
}

void openniSoundWave::update()
{
    //OPENNI
    if( !V::OpenNIDeviceManager::USE_THREAD )
    {
        _manager->update();
    }

    if (showUser){
	// Uses manager to handle users.
    for( std::map<int, gl::Texture>::iterator it=mUsersTexMap.begin(); it != mUsersTexMap.end(); ++it )
    {
        it->second = getUserImage( it->first ); 
    }
    } else {
        // Update textures
        mColorTex = getColorImage();
        mDepthTex = getDepthImage();
    }
		
	mEye = Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
    
    mPcmBuffer = mInput.getPcmBuffer();
    if( ! mPcmBuffer ) {
        return;
    }
    
    bandCount = 125;
    mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );

    updateWave();

}

void openniSoundWave::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ), true );
	
	gl::pushMatrices();
    glPointSize(waveWidth);
		gl::scale( Vec3f( -1.0f, -1.0f, 1.0f ) );
		gl::rotate( mSceneRotation );
    
    if (showUser){    
    for( std::map<int, gl::Texture>::iterator it=mUsersTexMap.begin(); it != mUsersTexMap.end();++it )
    {
        //int id = it->first;
        gl::Texture tex = it->second;
        tex.bind(0);

    }
    } else {
        mDepthTex.bind( 0 );
    }
        audioTexture.bind(1);
		mShader.bind();
		mShader.uniform("depthTex", 0 );
        mShader.uniform("soundTex", 1 );
        mShader.uniform("waveAmplitude", waveAmplitude);
		gl::draw( mVboMesh );
		mShader.unbind();
	gl::popMatrices();
    
    mScreenSyphon.publishScreen(); //publish the screen
	//mTextureSyphon.publishTexture(&mTex); //publish our texture
    params::InterfaceGl::draw();
    
    gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(132.0f, 52.0f),Color::white(),mFont);


}

void openniSoundWave::onNewUser( V::UserEvent event )
{
	app::console() << "New User Added With ID: " << event.mId << std::endl;
    mUsersTexMap.insert( std::make_pair( event.mId, gl::Texture(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT) ) );
}


void openniSoundWave::onLostUser( V::UserEvent event )
{
	app::console() << "User Lost With ID: " << event.mId << std::endl;
    mUsersTexMap.erase( event.mId );
}



CINDER_APP_BASIC( openniSoundWave, RendererGl )
