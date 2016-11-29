#include "cinder/app/AppBasic.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ArcBall.h"
#include "cinder/Quaternion.h"
#include "cinder/Camera.h"
#include "cinder/Capture.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "cinderSyphon.h"

#include "cinder/params/Params.h"
#include "cinder/Easing.h"
#include "cinder/ip/Resize.h"

#define WIDTH 1024
#define HEIGHT 900
const float TWEEN_SPEED = 0.08f;
using namespace ci;
using namespace ci::app;
using namespace std;

class videoToPartApp : public AppBasic {
public:
	void	prepareSettings( Settings *settings );
	void	setupTextures();
	void	resetFBOs();
	void	setupVBO();
	void	setup();
    void    setupCapture();
	void	update();
	void	draw();
	void    mouseDown( MouseEvent event );
	void    mouseDrag( MouseEvent event );
	void	keyDown( KeyEvent event );
    void    fileDrop( FileDropEvent event );
    void    updateEasying();
    void    resetView();
    void    updateCapture();
    
	CameraPersp		mCam;
	Arcball			mArcball;
	Surface32f		mInitPos, mInitVel;
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBO[2];
	gl::Texture		mPositions, mVelocities;
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mPosShader, mDisplShader;
    
    Surface32f      imgSurface;
    Surface         mSurface;

    ImageSourceRef  imgSurfaceRef;
    gl::Texture     imgTexture;
    float volume;
    bool showing;
    
	float transper;
    
	float easing;
    
    Capture		mCapture;
    
    audio::Input mInput;
	std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
    void audioSetup();
    void audioUpdate();
    
    bool isPositive;
    
    syphonServer mSyphonServer; //our syphon client
    
    // PARAMS
	params::InterfaceGl	mParams;

};

void videoToPartApp::setupCapture()
{
	try {
		mCapture = Capture( 640, 480 );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
}

void videoToPartApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1000, 800 );
	//settings->setFullScreen(true);
}

void videoToPartApp::audioUpdate()
{
	mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}
	uint16_t bandCount = 512;
	//presently FFT only works on OS X, not iOS
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
    float * fftBuffer = mFftDataRef.get();
    
    if (fftBuffer)
    volume = fftBuffer[0]*1.5;

}

void videoToPartApp::setupTextures(){
    
    Surface imgSurface;

	if( mCapture.isCapturing() && mSurface) {
        Area srcArea(0,0,mSurface.getWidth(), mSurface.getHeight());

        imgSurface = ip::resizeCopy(mSurface, srcArea, getWindowSize(), FilterBox() );
	} else {
        Area srcArea(0,0,imgSurfaceRef.get()->getWidth(), imgSurfaceRef.get()->getHeight());
        Surface srcSur(imgSurfaceRef);
       imgSurface = ip::resizeCopy(srcSur, srcArea, getWindowSize(), FilterBox() );
    }

    Channel imgChannel(imgSurface);
    float intns1 = imgChannel.areaAverage(Area(0,0,imgChannel.getWidth(),imgChannel.getHeight()));
    
	mInitPos = Surface32f( getWindowWidth(), getWindowHeight(), true);
	Surface32f::Iter pixelIter = mInitPos.getIter();
    std::size_t cnt = 0;
    float max = 0;
    while(cnt<getWindowWidth()*getWindowHeight()){
        Channel::Iter imgIter = imgChannel.getIter();
        while( imgIter.line()) {
            while( imgIter.pixel()) {
                /* Initial particle positions are passed in as R,G,B 
                 float values. Alpha is used as particle mass. */
                float x, y, z;
                max = imgIter.v()>max?imgIter.v():max;
                if ((imgIter.v())>(intns1+10)){
                    x = ((float)imgIter.getPos().x)/(float)getWindowWidth()-0.5f;
                    y = ((float)imgIter.getPos().y)/(float)getWindowHeight()-0.5f;
                    z = (Rand::randFloat()-0.5f)*0.001f;
                    cnt++;
                    mInitPos.setPixel( imgIter.getPos(), ColorAf( z, x, y , Rand::randFloat(0.2f, 1.0f) ) );
                }
            }
        }
    }
    
    std::cout<<max<<" "<<std::endl;
    
	gl::Texture::Format tFormat;
	tFormat.setInternalFormat(GL_RGBA32F_ARB);
	mPositions = gl::Texture( mInitPos, tFormat);
	mPositions.setWrap( GL_REPEAT, GL_REPEAT );
	mPositions.setMinFilter( GL_NEAREST );
	mPositions.setMagFilter( GL_NEAREST );
	
	//Velocity 2D texture array
	mInitVel = Surface32f( WIDTH, HEIGHT, true);
	pixelIter = mInitVel.getIter();
	while( pixelIter.line() ) {
		while( pixelIter.pixel() ) {
			/* Initial particle velocities are
			 passed in as R,G,B float values. */
			mInitVel.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}
	mVelocities = gl::Texture( mInitVel, tFormat);
	mVelocities.setWrap( GL_REPEAT, GL_REPEAT );
	mVelocities.setMinFilter( GL_NEAREST );
	mVelocities.setMagFilter( GL_NEAREST );
}

void videoToPartApp::resetFBOs(){

	mCurrentFBO = 0;
	mOtherFBO = 1;
	mFBO[0].bindFramebuffer();
	mFBO[1].bindFramebuffer();
	
	// Attachment 0 - Positions
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	gl::setMatricesWindow( mFBO[0].getSize(), false );
	gl::setViewport( mFBO[0].getBounds() );
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mPositions.enableAndBind();
	gl::draw( mPositions, mFBO[0].getBounds() );
	mPositions.unbind();
	
	// Attachment 1 - Velocities
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mVelocities.enableAndBind();
	gl::draw( mVelocities, mFBO[0].getBounds() );
	mVelocities.unbind();
	
	mFBO[1].unbindFramebuffer();
	mFBO[0].unbindFramebuffer();
	mPositions.disable();
	mVelocities.disable();
}

void videoToPartApp::setupVBO(){
	/*	A dummy VboMesh the same size as the
		texture to keep the vertices on the GPU */
	int totalVertices =  WIDTH * HEIGHT;
	vector<Vec2f> texCoords;
	vector<uint32_t> indices;
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	//layout.setDynamicColorsRGBA();
	mVboMesh = gl::VboMesh( totalVertices, totalVertices, layout, GL_POINTS);
	for( int x = 0; x < WIDTH; ++x ) {
		for( int y = 0; y < HEIGHT; ++y ) {	
			indices.push_back( x * WIDTH + y );
			texCoords.push_back( Vec2f( x/(float)WIDTH, y/(float)HEIGHT ) );
		}
	}
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
}

void videoToPartApp::audioSetup()
{
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

void videoToPartApp::setup()
{	
	gl::clear();
    
    isPositive = true;
	
//	mClientSyphon.setup();
//    
//	// in order for this to work, you must run simple server from the testapps directory
//	// any other syphon item you create would work as well, just change the name
//    mClientSyphon.setApplicationName("Simple Server");
//    mClientSyphon.setServerName("Gusev, Vladimir's iPhone - Airbeam");
//	
//	mClientSyphon.bind();

    
    setupCapture();
   
    imgSurfaceRef = loadImage( "/Users/eight/Desktop/rats-anim.gif" );
    volume = 0.0;
    transper = 0.5;
    mParams = params::InterfaceGl( "Sound Particles", Vec2i( 200, 180 ) );
	mParams.addParam( "Volume", &volume, "min=0.0 max=10.0 step=0.5 keyIncr=V keyDecr=v" );

	try {
		// Multiple render targets shader updates the positions/velocities
		mPosShader = gl::GlslProg( loadResource("pos.vert"), loadResource("pos.frag"));
		// Vertex displacement shader
		mDisplShader = gl::GlslProg( loadResource( "vDispl.vert" ), loadResource( "vDispl.frag" ));
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << endl;
	}
	setupTextures();
	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, 2);
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFBO[0] = gl::Fbo( WIDTH, HEIGHT, format );
	mFBO[1] = gl::Fbo( WIDTH, HEIGHT, format );
	resetFBOs();
	// THE VBO HAS TO BE DRAWN AFTER FBO!
	setupVBO();
    
    audioSetup();
}

void videoToPartApp::resetView(){
    mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( getWindowHeight() / 2.0f );
    mArcball.resetQuat();
	
	mCam.lookAt( Vec3f( 0.0f, 0.0f, -450.0f ), Vec3f::zero() );
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.1f, 2000.0f );
	gl::setMatrices( mCam );
}

void videoToPartApp::updateCapture(){
	if( mCapture && mCapture.checkNewFrame() ) {
		mSurface = mCapture.getSurface();
	}
}

void videoToPartApp::update()
{
    updateCapture();
    
	gl::setMatricesWindow( mFBO[0].getSize(), false ); // false to prevent vertical flipping
	gl::setViewport( mFBO[0].getBounds() );
	
	mFBO[ mCurrentFBO ].bindFramebuffer();
	
	GLenum buf[2] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
	glDrawBuffers(2, buf);
	mFBO[ mOtherFBO ].bindTexture(0, 0);
	mFBO[ mOtherFBO ].bindTexture(1, 1);
	mPosShader.bind();
	mPosShader.uniform("posArray", 0 );
	mPosShader.uniform("velArray", 1 );
    mPosShader.uniform("volume", volume);
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0.0f, 0.0f); glVertex2f( 0.0f, 0.0f);
	glTexCoord2f( 0.0f, 1.0f); glVertex2f( 0.0f, HEIGHT);
	glTexCoord2f( 1.0f, 1.0f); glVertex2f(  WIDTH, HEIGHT);
	glTexCoord2f( 1.0f, 0.0f); glVertex2f( WIDTH, 0.0f);
	glEnd();
	
	mPosShader.unbind();
	mFBO[ mOtherFBO ].unbindTexture();
	mFBO[ mCurrentFBO ].unbindFramebuffer();
	
	mCurrentFBO = ( mCurrentFBO + 1 ) % 2;
	mOtherFBO   = ( mCurrentFBO + 1 ) % 2;
    
    updateEasying();
    audioUpdate();
    
    //Surface sySurface = Surface(*(mClientSyphon.mTex));
}

void videoToPartApp::draw()
{
	gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
	gl::clear(isPositive? ColorA( 0.0f, 0.0f, 0.0f, 1.0f ) : ColorA( 1.0f, 1.0f, 1.0f, 1.0f ));
    glPointSize(1.0+volume*0.01);
	mFBO[mCurrentFBO].bindTexture(0,0);
	mDisplShader.bind();
    mDisplShader.uniform("isNegative", isPositive);
	mDisplShader.uniform("displacementMap", 0 );
	gl::pushModelView();
    gl::rotate( mArcball.getQuat() );
    gl::rotate(Vec3f(90.0,1.0,90.0f));
    gl::draw( mVboMesh );
    gl::popModelView();
	
	mDisplShader.unbind();
	mFBO[mCurrentFBO].unbindTexture();
    
    gl::setMatrices( mCam );
	gl::setViewport( getWindowBounds() );
    
    gl::setMatricesWindow(getWindowSize());
	gl::drawString( toString(  WIDTH * HEIGHT ) + " particles", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
    
    if (mSurface){
        gl::draw(gl::Texture(mSurface), Rectf(0,0,100,100));
    }
    //params::InterfaceGl::draw();

}


void videoToPartApp::mouseDown( MouseEvent event )
{
    mArcball.mouseDown( event.getPos() );
}

void videoToPartApp::mouseDrag( MouseEvent event )
{
    mArcball.mouseDrag( event.getPos() );
}

void videoToPartApp::keyDown( KeyEvent event ){
	if( event.getChar() == 'r' ) {
        setupTextures();
		resetFBOs();
        resetView();
	}
    if (event.getChar() == ' '){
        ( mCapture && mCapture.isCapturing() ) ? mCapture.stop() : mCapture.start();
    }
    
    if (event.getChar() == 'c'){
        if (mCapture && mCapture.isCapturing()){
            mSurface = mCapture.getSurface();
        }
    }
    
    if (event.getChar() == 'n'){
        isPositive = !isPositive;
    }
}

void videoToPartApp::fileDrop( FileDropEvent event )
{
	try {
		imgSurfaceRef = loadImage( event.getFile( 0 ) );
        if (mCapture){
            mCapture.stop();
        }
        setupTextures();
        resetFBOs();
	}
	catch( ... ) {
		console() << "unable to load the texture file!" << std::endl;
	};
}

void videoToPartApp::updateEasying()
{
	//if (transper == 1.0f) return;
	double ff = fmod( getElapsedSeconds() * TWEEN_SPEED, 1 );
	float time = math<float>::clamp( ff * 1.5f, 0, 1 );
    transper = easeInExpo(time);
}

CINDER_APP_BASIC( videoToPartApp, RendererGl )