#include "cinder/app/AppBasic.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Arcball.h"
#include "cinder/Quaternion.h"
#include "cinder/Camera.h"
#include "cinder/Capture.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "cinder/ip/Flip.h"
#include "cinder/Xml.h"

#include "cinder/params/Params.h"
#include "cinder/Easing.h"
#include "cinder/ip/Resize.h"
#include "cinderSyphon.h"

#include "OscListener.h"

#define WIDTH 900
#define HEIGHT 600

using namespace ci;
using namespace ci::app;
using namespace std;
 
class img2Parts : public AppBasic {


public:
	void	prepareSettings( Settings *settings );
	void 	putImage(ImageSourceRef ref);
	void	setupTextures();
	void    setupWords();
	void	resetFBOs();
	void	setupVBO();
	void	setup();
	void	resize();
	void	update();
	void	draw();
	void    mouseDown( MouseEvent event );
	void    mouseDrag( MouseEvent event );
	void	keyDown( KeyEvent event );
    void    fileDrop( FileDropEvent event );
    
	CameraPersp		mCam;
	Arcball			mArcball;
	Surface32f		mInitPos, mInitVel;
	int				mCurrentFBO;
	int				mOtherFBO;
	gl::Fbo			mFBO[2];
	gl::Texture		mPositions, mVelocities;
	gl::Texture		mTexture;
	gl::VboMesh		mVboMesh;
	gl::GlslProg	mPosShader, mDisplShader, imgShader;
	gl::GlslProg    zoomShader;
    
    Surface32f      imgSurface;
    vector<Surface32f> words;
    vector<gl::Texture> velocities;
    vector<gl::Texture> positions;

    ImageSourceRef  imgSurfaceRef;
    float volume;
    bool showing;
    bool isCursorShown;
    bool isNegative;
    bool isParticleNeg;
    
	float easing;
    
    audio::Input mInput;
	std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
    void audioSetup();
    void audioUpdate();
    
//    syphonServer mTextureServer; //our syphon client
    // PARAMS
	params::InterfaceGl	mParams;
    
    syphonServer sServer;
    
    osc::Listener 	listener;
    bool mStart;
    u_long mStartFrames;

};

void img2Parts::prepareSettings( Settings *settings )
{
	settings->setWindowSize( WIDTH, HEIGHT );
    settings->setResizable(true);
}

void img2Parts::audioUpdate()
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

void img2Parts::setupTextures(){

    Channel imgChannel(imgSurface);
    
	mInitPos = Surface32f( WIDTH, HEIGHT, true);

    std::size_t cnt = 0;
    float oneOverWidth = -1.0 / (float)WIDTH;
    float oneOverHeight = 1.0 / (float)HEIGHT;
    while(cnt<HEIGHT*WIDTH){
        Channel::Iter imgIter = imgChannel.getIter();
        while( imgIter.line()) {
            while( imgIter.pixel()) {
                /* Initial particle positions are passed in as R,G,B 
                 float values. Alpha is used as particle mass. */
                float x, y, z, value;
                value = imgIter.v();
                if ((isParticleNeg && (value)<100.0f) || (!isParticleNeg && (value)>100.0f)){
                    x = ((float)imgIter.getPos().x)*oneOverWidth+0.5f;
                    y = ((float)imgIter.getPos().y)*oneOverHeight-0.5f;
                    z = (Rand::randFloat()-0.5f)*0.001f;
                    cnt++;
                    mInitPos.setPixel( imgIter.getPos(), ColorAf( x, y, z , Rand::randFloat(0.2f, 1.0f) ) );
                }
            }
        }
    }
    
	gl::Texture::Format tFormat;
	tFormat.setInternalFormat(GL_RGBA32F_ARB);
	mPositions = gl::Texture( mInitPos, tFormat);
	mPositions.setWrap( GL_REPEAT, GL_REPEAT );
	mPositions.setMinFilter( GL_NEAREST );
	mPositions.setMagFilter( GL_NEAREST );
	
	//Velocity 2D texture array
	mInitVel = Surface32f( WIDTH, HEIGHT, true);
	Surface32f::Iter pixelIter = mInitVel.getIter();
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

	velocities.push_back(mVelocities);
	positions.push_back(mPositions);

//	mTexture = gl::Texture( loadImage( loadResource( "nebula.jpg" ) ) );
//	mTexture.setWrap( GL_REPEAT, GL_REPEAT );
//	mTexture.setMinFilter( GL_NEAREST );
//	mTexture.setMagFilter( GL_NEAREST );
}

void img2Parts::resetFBOs(){

	mCurrentFBO = 0;
	mOtherFBO = 1;
	mFBO[0].bindFramebuffer();
	mFBO[1].bindFramebuffer();
	
	// Attachment 0 - Positions
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	gl::setMatricesWindow( mFBO[0].getSize(), false );
	gl::setViewport( mFBO[0].getBounds() );
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mPositions.enableAndBind();
	gl::draw( mPositions, mFBO[0].getBounds() );
	mPositions.unbind();
	
	// Attachment 1 - Velocities
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT);
	mVelocities.enableAndBind();
	gl::draw( mVelocities, mFBO[0].getBounds() );
	mVelocities.unbind();
	
	mFBO[1].unbindFramebuffer();
	mFBO[0].unbindFramebuffer();
	mPositions.disable();
	mVelocities.disable();
}

void img2Parts::setupVBO(){
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

void img2Parts::audioSetup()
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

void img2Parts::setup()
{
	gl::clear();
    isCursorShown = true;
    isNegative = true;
    isParticleNeg = false;
    setupWords();
    imgSurfaceRef = words[0];
    volume = 0.0;

    mParams = params::InterfaceGl( "Sound Particles", Vec2i( 200, 180 ) );
	mParams.addParam( "Volume", &volume, "min=0.0 max=10.0 step=0.5 keyIncr=V keyDecr=v" );
	try {
		// Multiple render targets shader updates the positions/velocities
		mPosShader = gl::GlslProg( loadResource("pos.vert"), loadResource("wind.frag"));
		// Vertex displacement shader
		mDisplShader = gl::GlslProg( loadResource( "vDispl.vert" ), loadResource( "vDispl.frag" ));

        //starting image transperancy
        imgShader = gl::GlslProg( loadResource( "img.vert" ), loadResource( "img.frag" ));

        //zoomShader = gl::GlslProg(loadResource("zoom.vert"), loadResource("zoom.frag"));
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
    mCurrentFBO = 0;
	mOtherFBO = 1;
	resetFBOs();
	// THE VBO HAS TO BE DRAWN AFTER FBO!
	setupVBO();
    
    audioSetup();
    
    sServer.setName("OPTIMISTIC");
    
    listener.setup( 3000 );
    
    mStart = false;
    mStartFrames = 0;
}

void img2Parts::resize()
{
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( getWindowHeight() / 2.0f );
	
	mCam.lookAt( Vec3f( 0.0f, 0.0f, -450.0f ), Vec3f::zero() );
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.1f, 2000.0f );
}

void img2Parts::update()
{
	//gl::setMatricesWindow( mFBO[0].getSize() ); // false to prevent vertical flipping
	//gl::setViewport( mFBO[0].getBounds() );
    uint32_t elapsed = getElapsedFrames();
    if ( mStart || ( elapsed - mStartFrames < 2 )  ) {
        mFBO[ mCurrentFBO ].bindFramebuffer();
        
        GLenum buf[2] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
        glDrawBuffers(2, buf);
        mFBO[ mOtherFBO ].bindTexture(0, 0);
        mFBO[ mOtherFBO ].bindTexture(1, 1);
        mPosShader.bind();
        mPosShader.uniform( "posArray", 0 );
        mPosShader.uniform( "velArray", 1 );
        mPosShader.uniform( "volume", volume);

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
    }

    audioUpdate();
    
    while( listener.hasWaitingMessages() ) {
        osc::Message message;
        listener.getNextMessage( &message );
        if (message.getNumArgs() != 1) break;
        
        if( message.getArgType(0) == osc::TYPE_FLOAT ) {
            float startStop = message.getArgAsFloat(0);
            if (startStop == 1) {
                mStart = true;
                mStartFrames = getElapsedFrames();
            } else {
                mStart = false;
            }
        }
    }
}

void img2Parts::draw()
{

	gl::setMatrices( mCam );

	//gl::setViewport( getWindowBounds() );
    gl::clear(isNegative? ColorA( 0.0f, 0.0f, 0.0f, 1.0f ) : ColorA( 1.0f, 1.0f, 1.0f, 1.0f ));
	mFBO[mCurrentFBO].bindTexture(0,0);

	mDisplShader.bind();
    mDisplShader.uniform("isNegative", isNegative);
	mDisplShader.uniform("displacementMap", 0 );

	gl::pushModelView();
        float size = math<float>::clamp(volume, 1.0, 2.0);
        gl::scale(Vec3f(1.0f,1.0f,math<float>::clamp(volume, 1.0, 1.05)));
        size = (Rand::randFloat(size));
        //glPointSize(size);

        gl::rotate( mArcball.getQuat() );
        gl::draw( mVboMesh );
    gl::popModelView();
	

	mDisplShader.unbind();

	mFBO[mCurrentFBO].unbindTexture();

    sServer.publishScreen();
    
    gl::setMatricesWindow(getWindowSize());
    
//    mTextureServer.publishScreen(); //publish our texture
	gl::drawString( toString(  WIDTH * HEIGHT ) + " particles", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
    
    //params::InterfaceGl::draw();
}


void img2Parts::mouseDown( MouseEvent event )
{
    mArcball.mouseDown( event.getPos() );
}

void img2Parts::mouseDrag( MouseEvent event )
{
    mArcball.mouseDrag( event.getPos() );
}

void img2Parts::keyDown( KeyEvent event ){
	if( event.getChar() == 'r' ) {
		resetFBOs();
        if (event.isShiftDown()){
            mArcball.resetQuat();
            mCam.setFov(60.0f);
        }
	}
    if (event.getChar() == 'n'){
        isNegative = !isNegative;
    }
    if (event.getChar() == 'b'){
    	getWindow()->setBorderless(!getWindow()->isBorderless());
    }
    
    if (event.getChar() == ' '){
        isParticleNeg = !isParticleNeg;
    }
    
    if (event.getChar() == 'c'){
        isCursorShown = !isCursorShown;
        if (isCursorShown)showCursor();
        else hideCursor();
    }
    
    if (event.getChar() == '-'){
        mCam.setFov(math<float>::clamp(mCam.getFov()+1.0, 0.0f,180.0f));
    }
    
    if (event.getChar() == '+'){
        mCam.setFov(math<float>::clamp(mCam.getFov()-1.0, 0.0f,180.0f));
    }
    
    if (event.getChar() == 's') {
        mStart = true;
    }
    
    if (event.getChar() == 'S') {
        mStart = false;
    }

    try {
		uint num = boost::lexical_cast<uint>(event.getChar());

		num = math<uint>::clamp(num, 0, words.size()-1);
		imgSurface = words[num];
		mVelocities = velocities[num];
		mPositions = positions[num];
        resetFBOs();
    } catch (boost::bad_lexical_cast e){

    }
}

void img2Parts::putImage(ImageSourceRef ref){

    Surface32f imgSurface = ip::resizeCopy( Surface32f(ref), Area(0,0,ref.get()->getWidth(), ref.get()->getHeight()), Vec2i(WIDTH, HEIGHT ), FilterSincBlackman() );
    ip::flipVertical(&imgSurface);
	words.push_back(imgSurface);
}

void img2Parts::fileDrop( FileDropEvent event ){
	words.clear();
    velocities.clear();
    positions.clear();
	putImage(loadImage( loadResource( "Skeleton_01_by_pelleron.png" )));
    imgSurface = words.back();
    setupTextures();
    resetFBOs();
	for (uint i = 0; i < event.getNumFiles(); i++) {
		try {
			putImage(loadImage(event.getFile(i)));
			imgSurface = words.back();
			setupTextures();
			resetFBOs();
		} catch (...) {
			console() << "unable to load the texture file!" << std::endl;
		};
	}

}

CINDER_APP_BASIC( img2Parts, RendererGl)

inline void img2Parts::setupWords() {
   putImage(loadImage( loadResource( "Skeleton_01_by_pelleron.png" )));
   imgSurface = words.back();
}

