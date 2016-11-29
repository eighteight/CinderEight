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
#include "cinder/Utilities.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "cinder/ip/Flip.h"
#include "cinder/Xml.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/params/Params.h"
#include "cinder/Easing.h"
#include "cinder/ip/Resize.h"

#define WIDTH 900
#define HEIGHT 600
const float TWEEN_SPEED = 0.08f;
using namespace ci;
using namespace ci::app;
using namespace std;
 
class video2Particles : public AppBasic {

public:
	void	prepareSettings( Settings *settings );
	void	setupTextures();
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
    void    loadMovieFile( const fs::path &moviePath );
    
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

    ImageSourceRef  imgSurfaceRef;
    float volume;
    bool showing;
    bool isCursorShown;
    bool isNegative;
    bool isParticleNeg;
    bool doParticles;
    
	float easing;
    
    audio::Input mInput;
	std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
    void audioSetup();
    void audioUpdate();
    
//    syphonServer mTextureServer; //our syphon client
    // PARAMS
	params::InterfaceGl	mParams;
    
    qtime::MovieGlRef		mMovieGl;
	qtime::MovieSurface     mMovie;
    Surface				    mSurface;
    gl::Texture				mFrameTexture;

};

void video2Particles::prepareSettings( Settings *settings )
{
	settings->setWindowSize( WIDTH, HEIGHT );
    settings->setResizable(true);
}

void video2Particles::audioUpdate()
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

void video2Particles::setupTextures(){

    if (!mSurface) return;

    Channel imgChannel(mSurface);
    
	mInitPos = Surface32f( WIDTH, HEIGHT, true);
	Surface32f::Iter pixelIter = mInitPos.getIter();
    std::size_t cnt = 0;
    float oneOverWidth = 1.0 / (float)imgChannel.getWidth();
    float oneOverHeight = 1.0 / (float)imgChannel.getHeight();
    while(cnt<HEIGHT*WIDTH){
        Channel::Iter imgIter = imgChannel.getIter();
        while( imgIter.line()) {
            while( imgIter.pixel()) {
                /* Initial particle positions are passed in as R,G,B 
                 float values. Alpha is used as particle mass. */
                float x, y, z;
                if ((isParticleNeg && (imgIter.v())<100) || (!isParticleNeg && (imgIter.v())>100)){
                    x = ((float)imgIter.getPos().x)*oneOverWidth - 0.5f;
                    y = ((float)imgIter.getPos().y)*oneOverHeight- 0.5f;
                    z = (Rand::randFloat()-0.5f)*0.001f;
                    cnt++;
                    mInitPos.setPixel( imgIter.getPos(), ColorAf( x, y, 0.0 , Rand::randFloat(0.2f, 1.0f) ) );
                }
            }
        }
    }
    
    cout <<cnt<<" "  << imgChannel.getSize().x*imgChannel.getSize().y<<endl;
    
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
			/* Initial particle velocities are passed in as R,G,B float values. */
			mInitVel.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}
	mVelocities = gl::Texture( mInitVel, tFormat);
	mVelocities.setWrap( GL_REPEAT, GL_REPEAT );
	mVelocities.setMinFilter( GL_NEAREST );
	mVelocities.setMagFilter( GL_NEAREST );
}

void video2Particles::resetFBOs(){

	mCurrentFBO = 0;
	mOtherFBO = 1;
	mFBO[0].bindFramebuffer();
    
	
	// Attachment 0 - Positions
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	gl::setMatricesWindow( mFBO[0].getSize());
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

void video2Particles::setupVBO(){
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

void video2Particles::audioSetup()
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

void video2Particles::setup()
{

	gl::clear();
    isCursorShown = true;
    isNegative = true;
    isParticleNeg = false;
    
    doParticles = false;

    volume = 0.0;

    mParams = params::InterfaceGl( "Bedrunken Particles", Vec2i( 200, 180 ) );
	mParams.addParam( "Volume", &volume, "min=0.0 max=10.0 step=0.5 keyIncr=V keyDecr=v" );
	try {
		// Multiple render targets shader updates the positions/velocities
		mPosShader = gl::GlslProg( loadResource("pos.vert"), loadResource("pos.frag"));
		// Vertex displacement shader
		mDisplShader = gl::GlslProg( loadResource( "vDispl.vert" ), loadResource( "vDispl.frag" ));

        //starting image transparancy
        imgShader = gl::GlslProg( loadResource( "img.vert" ), loadResource( "img.frag" ));

	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << endl;
	}

	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, 2);
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
	mFBO[0] = gl::Fbo( WIDTH, HEIGHT, format );
	mFBO[1] = gl::Fbo( WIDTH, HEIGHT, format );
    
    audioSetup();
    
    loadMovieFile( fs::path("/Users/eight/Desktop/bedrunkene/rects/rects.mov") );
    doParticles = false;

}

void video2Particles::resize()
{
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth()  *.5f, getWindowHeight()  *.5f ) );
	mArcball.setRadius( getWindowHeight() *.5f );
	
	mCam.lookAt( Vec3f( 0.0f, 0.0f, getWindowWidth()  *.5f ), Vec3f::zero() );
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .001f, 2000.0f );
}

void video2Particles::update()
{
    if( mMovie ){
		mSurface = mMovie.getSurface();
    }
    
    if (!doParticles) return;

	gl::setMatricesWindow( mFBO[0].getSize()); // false to prevent vertical flipping
	gl::setViewport(mFBO[0].getBounds() );
	
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

    audioUpdate();
}

void video2Particles::draw()
{
    if( mSurface && !doParticles) {
		gl::draw( gl::Texture( mSurface ),getWindowBounds() );
        return;
	}
    
    if (!doParticles) {
        //gl::setMatricesWindow(getWindowSize(), false);
        return;
    }

    gl::setMatrices( mCam );
    gl::setMatricesWindow(getWindowSize());
    gl::setViewport( getWindowBounds() );
    gl::clear(isNegative? ColorA( 0.0f, 0.0f, 0.0f, 1.0f ) : ColorA( 1.0f, 1.0f, 1.0f, 1.0f ));
	mFBO[mCurrentFBO].bindTexture(0,0);

	mDisplShader.bind();
    mDisplShader.uniform("isNegative", isNegative);
	mDisplShader.uniform("displacementMap", 0 );

	gl::pushModelView();
        float size = math<float>::clamp(volume, 1.0, 2.0);
        gl::scale(Vec3f(1.0f,1.0f,math<float>::clamp(volume, 1.0, 1.02)));
        size = (Rand::randFloat(size));
        glPointSize(size);

        gl::rotate( mArcball.getQuat() );
        gl::draw( mVboMesh );
    gl::popModelView();

	mDisplShader.unbind();

	mFBO[mCurrentFBO].unbindTexture();

    gl::setMatricesWindow(getWindowSize());
    
//    mTextureServer.publishScreen(); //publish our texture
	gl::drawString( toString(  WIDTH * HEIGHT ) + " particles", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
    
    //params::InterfaceGl::draw();
}


void video2Particles::mouseDown( MouseEvent event )
{
    mArcball.mouseDown( event.getPos() );
}

void video2Particles::mouseDrag( MouseEvent event )
{
    mArcball.mouseDrag( event.getPos() );
}

void video2Particles::keyDown( KeyEvent event ){
	if( event.getChar() == 'r' ) {
        setupTextures();
		resetFBOs();
        mArcball.resetQuat();
        mCam.setFov(60.0f);
        doParticles = !doParticles;
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
    
    if (event.getChar() == 'm'){
        doParticles = false;
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

    try {
		uint num = boost::lexical_cast<uint>(event.getChar());
		//imgSurface = Surface32f(mFrameTexture);
        setupTextures();
        resetFBOs();
        // THE VBO HAS TO BE DRAWN AFTER FBO!
        setupVBO();
        doParticles = true;
    } catch (boost::bad_lexical_cast e){

    }
}

void video2Particles::fileDrop( FileDropEvent event ){
    
    fs::path moviePath = event.getFile(0);
	if( ! moviePath.empty() ){
		loadMovieFile( moviePath );
        doParticles = false;
    }
}

void video2Particles::loadMovieFile( const fs::path &moviePath )
{
    try {
        // load up the movie, set it to loop, and begin playing
        mMovie = qtime::MovieSurface( moviePath );//qtime::MovieGl::create( moviePath );
        mMovie.setLoop();
        mMovie.play();

    }
    catch( ... ) {
        console() << "Unable to load the movie." << std::endl;
        mMovie.reset();
    }
    
    mFrameTexture.reset();
}

CINDER_APP_BASIC( video2Particles, RendererGl)

