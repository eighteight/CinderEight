#include "cinder/app/AppBasic.h"
#include "cinder/Vector.h"
#include "cinder/ImageIO.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "ParticleController.h"
#include "cinder/gl/TextureFont.h"
#include <string>

#define NUM_INITIAL_PARTICLES 1
#define NUM_PARTICLES_TO_SPAWN 100

#define MAX_PARTICLES	1100000

enum DrawMode {
    kModeNormal,
    kModeVA,
    kModeVBO
};

using namespace ci;
using namespace ci::app;

static const std::string  supportedchars = /*gl::TextureFont::defaultChars()+*/" яшертыуиопасдфгчйклзхцвбнмюжьъЯШЕРТЫУИОПЮЖЭАСДФГЧЙКЛ:\"ЗХЦВБМН<>?";

class FlockingApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void	mouseDown( MouseEvent event );
	void	mouseUp( MouseEvent event );
	void	mouseWheel( MouseEvent event );
	void	mouseMove( MouseEvent event );
	void	mouseDrag( MouseEvent event );
    
	void setup();
	void update();
	void draw();
	
	// PARAMS
	params::InterfaceGl	mParams;
	
	// CAMERA
	CameraPersp			mCam;
	Quatf				mSceneRotation;
	Vec3f				mEye, mCenter, mUp;
	float				mCameraDistance;
	
	ParticleController	mParticleController;
	float				mZoneRadius;
	float				mLowerThresh, mHigherThresh;
	float				mAttractStrength, mRepelStrength, mOrientStrength;
	
	bool				mCentralGravity;
	bool				mFlatten;
	bool				mSaveFrames;
	cinder::gl::TextureFontRef	textureFontRef;
	Font				mFont;
	std::string             	wrd;
    
    //sprites
    float	pos[MAX_PARTICLES][3];
    float	vel[MAX_PARTICLES][3];
    float	col[MAX_PARTICLES][4];
    
    int			numParticles;
    int			adder;
    float		currentRot;
    DrawMode    drawMode;
    bool		bPause;
    bool		useTexture;
    bool		useSprite;
    bool		vsync	;
    bool		vboInited;
    int			pointSize;
    
    GLuint		vbo[2];

    
};

void FlockingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 60.0f );
}

void FlockingApp::setup()
{	
	Rand::randomize();
	
	mCentralGravity = true;
	mFlatten		= false;
	mSaveFrames		= false;
	
	mZoneRadius		= 80.0f;
	mLowerThresh	= 0.4f;
	mHigherThresh	= 0.75f;
	mAttractStrength	= 0.005f;
	mRepelStrength		= 0.01f;
	mOrientStrength		= 0.01f;
	
	// SETUP CAMERA
	mCameraDistance = 500.0f;
	mEye			= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter			= Vec3f::zero();
	mUp				= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 2000.0f );
    
	// SETUP PARAMS
	mParams = params::InterfaceGl( "Text Flocking", Vec2i( 200, 300 ) );
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams.addSeparator();
	mParams.addParam( "Eye Distance", &mCameraDistance, "min=50.0 max=1000.0 step=50.0" );
	mParams.addParam( "Center Gravity", &mCentralGravity);
	mParams.addParam( "Flatten", &mFlatten);
	mParams.addSeparator();
	mParams.addParam( "Zone Radius", &mZoneRadius, "min=10.0 max=100.0 step=1.0" );
	mParams.addParam( "Lower Thresh", &mLowerThresh, "min=0.025 max=1.0 step=0.025" );
	mParams.addParam( "Higher Thresh", &mHigherThresh, "min=0.025 max=1.0 step=0.025" );
	mParams.addSeparator();
	mParams.addParam( "Attract Strength", &mAttractStrength, "min=0.001 max=0.1 step=0.001" );
	mParams.addParam( "Repel Strength", &mRepelStrength, "min=0.001 max=0.1 step=0.001" );
	mParams.addParam( "Orient Strength", &mOrientStrength, "min=0.001 max=0.1 step=0.001" );
	
	// CREATE PARTICLE CONTROLLER
	mParticleController.addParticles( NUM_INITIAL_PARTICLES );
    
#if defined( CINDER_COCOA_TOUCH )
	mFont = Font( "Cochin-Italic", 24 );
#elif defined( CINDER_COCOA )
	mFont = Font( "Times New Roman", 24 );
#else
	mFont = Font( "Times New Roman", 24 );
#endif
	textureFontRef = gl::TextureFont::create( mFont, gl::TextureFont::Format(), supportedchars );
    
	wrd = "пушкин";
    //wrd = "pushking";
    
    
    ////////////////////// sprites
    numParticles		= 500000;
    adder				= 10000;
    currentRot			= 0;
    drawMode            = kModeNormal;
    bPause				= false;
    useTexture			= true;
    useSprite			= true;
    vsync				= false;
    vboInited			= false;
    pointSize			= 16;

	
	for(int i=0; i<MAX_PARTICLES; i++) {
		pos[i][0] = 125.0f;//ofRandom(-ofGetWidth(), ofGetWidth());
		pos[i][1] = 325.0f;//ofRandom(-ofGetHeight(), 2 * ofGetHeight());
		pos[i][2] = 125.0f;//ofRandom(-ofGetWidth(), ofGetWidth());
        
		vel[i][0] = 0.5f;//ofRandom(-1, 1);
		vel[i][1] = 0.4f;//ofRandom(-1, 1);
		vel[i][2] = 0.24f;//ofRandom(-1, 1);
        
		col[i][0] = 1;//ofRandom(0, 1);
		col[i][1] = 1;//ofRandom(0, 1);
		col[i][2] = 1;//ofRandom(0, 1);
	}
	
//	ofDisableArbTex();				// new in OF006, force any texture created from this point on, to be GL_TEXTURE_2D (normalized uv coords)
//	image.loadImage("grad.png");

//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glBindTexture(GL_TEXTURE_2D, textureFontRef->get()->);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
    
}

void FlockingApp::keyDown( KeyEvent event )
{
    
	switch( event.getChar() ) {
		case 'p':
			mParticleController.addParticles( NUM_PARTICLES_TO_SPAWN );
			break;
		case 'P':
			mParticleController.removeParticles( NUM_PARTICLES_TO_SPAWN );
			break;
		case '\r':
			wrd = "";
			break;
		case '=':
		case '+':
			mFont = Font( mFont.getName(), mFont.getSize() + 1 );
			textureFontRef = gl::TextureFont::create( mFont, gl::TextureFont::Format(), supportedchars );
            break;
		case '-':
			mFont = Font( mFont.getName(), mFont.getSize() - 1 );
			textureFontRef = gl::TextureFont::create( mFont, gl::TextureFont::Format(), supportedchars );
            break;
		case ' ':
			mSaveFrames = !mSaveFrames;
			break;
		case '\177':
			wrd = wrd.substr(0,wrd.length()-1);
			break;
		default:
			wrd.push_back(event.getChar());
            break;
	}
}


void FlockingApp::update()
{
	if( mLowerThresh > mHigherThresh ) mHigherThresh = mLowerThresh;
	
	// UPDATE CAMERA
    //	mEye = Vec3f( 0.0f, 0.0f, mCameraDistance );
    //	mCam.lookAt( mEye, Vec3f::zero(), mUp );
    //	gl::setMatrices( mCam );
    //	gl::rotate( mSceneRotation );
	
	// UPDATE PARTICLE CONTROLLER AND PARTICLES
	mParticleController.applyForce( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
	if( mCentralGravity ) mParticleController.pullToCenter( mCenter );
	mParticleController.update( mFlatten, &wrd );
}

inline void FlockingApp::mouseDown(MouseEvent event)
{
	mCenter.x = event.getX();
	mCenter.y = event.getY();
}

inline void FlockingApp::mouseUp(MouseEvent event)
{
}

inline void FlockingApp::mouseWheel(MouseEvent event)
{
}

inline void FlockingApp::mouseMove(MouseEvent event)
{
}

inline void FlockingApp::mouseDrag(MouseEvent event)
{
}

void FlockingApp::draw()
{	
	gl::clear( Color( 0, 0, 0 ), true );
    
    gl::enableAlphaBlending();
    
	gl::clear( Color( 0, 0, 0 ) );
    
    //	gl::enableDepthRead();
    //	gl::enableDepthWrite();
    
	// DRAW PARTICLES
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
	mParticleController.draw(&textureFontRef);
	
	gl::pushModelView();
    gl::color( ColorA( 1.0f, 0.25f, 0.25f, 1.0f ) );
    gl::drawSolidCircle( Vec2f(mCenter.x,mCenter.y), mZoneRadius );
    // Draw FPS
	gl::color( Color::white() );
	textureFontRef->drawString( toString (mParticleController.mNumParticles)+"ps. "+toString( floor(getAverageFps()) ) + " FPS", Vec2f( getWindowWidth()-300, getWindowHeight() - textureFontRef->getDescent() ) );
	gl::popModelView();
    
	if( mSaveFrames ){
		writeImage( getHomeDirectory() + "flocking/image_" + toString( getElapsedFrames() ) + ".png", copyWindowSurface() );
	}
	
	// DRAW PARAMS WINDOW
	params::InterfaceGl::draw();
    
}

CINDER_APP_BASIC( FlockingApp, RendererGl )
