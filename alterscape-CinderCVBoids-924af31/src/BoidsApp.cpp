#include "cinder/app/AppBasic.h"
#include "cinder/Vector.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIO.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "BoidController.h"
#include "SilhouetteDetector.h"
#include "time.h"
#include "cinder/Surface.h"
#include "Resources.h"




//CV stuff
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCV.h"
#include "BoidSysProperties.h"

#include <vector>

#define NUM_INITIAL_PARTICLES 100
#define NUM_PARTICLES_TO_SPAWN 15

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

class BoidsApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void setup();
	void update();
	void drawCapture();
	void draw();
	bool checkTime();
	void drawPolyLines();
	
	//Mouse code ///
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void mouseUp( MouseEvent event );
	void updateMousePosition(MouseEvent event);
	////
	
	// PARAMS
	params::InterfaceGl	mParams;
	
	// CAMERA
	CameraPersp			mCam;
	Quatf				mSceneRotation;
	Vec3f				mEye, mCenter, mUp;
	float				mCameraDistance;
	BoidController		flock_one;
	BoidController		flock_two;
	bool				mSaveFrames;
	bool				mIsRenderingPrint;
	double				changeInterval;
	time_t				lastChange;
	bool				gravity;
	int					newFlock;
	
	
	Capture				capture;
	gl::Texture			texture; // camera texture
	gl::Texture			mParticleTexture; // boid texture
	int					cvThreshholdLevel;
	Matrix44<float>		imageToScreenMap;

	
private:
	SilhouetteDetector	*silhouetteDetector;
	vector<Vec2i_ptr_vec> * polygons;
	vector<BoidSysPair> boidRulesets;
	int currentBoidRuleNumber;
	ci::ColorA imageColor;
	double lastFrameTime;
	double deltaT;
	bool shouldBeFullscreen;
};

void BoidsApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 875, 600 );
	settings->setFrameRate( 60.0f );
}

void BoidsApp::setup()
{	
	Rand::randomize();
	//setFullScreen(true);
	shouldBeFullscreen = false;
	
	mCenter				= Vec3f( getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f );
	mSaveFrames			= false;
	mIsRenderingPrint	= false;
	changeInterval		= 15.0;
	time(&lastChange);
	gravity				= false;
	
	// SETUP CAMERA
	mCameraDistance		= 350.0f;
	mEye				= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter				= Vec3f::zero();
	mUp					= Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 5000.0f );
	lastFrameTime		= getElapsedSeconds();
	deltaT				= 0;
	newFlock			= 0;
	
	mParticleTexture	= gl::Texture( loadImage( loadResource( RES_PARTICLE ) ) );
	
	
	// Initialize the OpenCV input (Below added RS 2010-11-15)
	try {
		//ci::Device device = Capture.
//		std::vector<boost::shared_ptr<Capture::Device> > devices = Capture::getDevices();
		
		std::cout << "Capture Devices:" << std::endl;
		std::vector<boost::shared_ptr<Capture::Device> >::iterator device_iterator;
		//for(device_iterator=devices.begin(); device_iterator!=devices.end(); device_iterator++)
//		for(int i=0; i<devices.size();i++)
//		{
//			console() << "device " << i << ": " << devices[i]->getName() << std::endl;
//			
//		}
		
		//UNCOMMENT FOLLOWING LINE FOR BUILT-IN CAMERA
		capture = Capture(320,240);
		
		//UNCOMMENT FOLLOWING LINE FOR UNIBRAIN OR WHATEVER
//		capture = Capture(320,240,devices[0]);//,devices[0]);	//FIXME this is a dumb way to select a device
		
		capture.start();
		silhouetteDetector = new SilhouetteDetector(320,240);
	} catch ( ... ) {
		console() << "Failed to initialize capture device" << std::endl;
	}
	cvThreshholdLevel = 45;		
	// CREATE PARTICLE CONTROLLER
	flock_one.addBoids( NUM_INITIAL_PARTICLES );
	flock_two.addBoids( NUM_INITIAL_PARTICLES );
	flock_one.setColor(ColorA( CM_RGB, 0.784, 0.0, 0.714, 1.0));
	flock_one.silRepelStrength = -0.50f;
	flock_two.setColor(ColorA( CM_RGB, 0.0, 1.0, 0.0, 1.0));
	imageColor = ColorA( CM_RGB, 0.4, 0.4, 0.4, 1.0);
	
	flock_one.addOtherFlock(&flock_two);
	flock_two.addOtherFlock(&flock_one);
	// SETUP PARAMS
	mParams = params::InterfaceGl( "Flocking", Vec2i( 200, 310 ) );
	mParams.addParam( "Scene Rotation", &mSceneRotation, "opened=1" );//
	mParams.addSeparator();
	mParams.addParam( "Fullscreen", &shouldBeFullscreen,"keyIncr=f" ); //FIXME
	mParams.addSeparator();
	mParams.addParam( "Eye Distance", &mCameraDistance, "min=100.0 max=2000.0 step=50.0 keyIncr=s keyDecr=w" );
	//mParams.addParam( "Center Gravity", &flock_one.centralGravity, "keyIncr=g" );
	//mParams.addParam( "Flatten", &flock_one.flatten, "keyIncr=f" );
	mParams.addSeparator();
	//mParams.addParam( "Zone Radius", &flock_one.zoneRadius, "min=10.0 max=100.0 step=1.0 keyIncr=z keyDecr=Z" );
//	mParams.addParam( "Lower Thresh", &flock_one.lowerThresh, "min=0.025 max=1.0 step=0.025 keyIncr=l keyDecr=L" );
//	mParams.addParam( "Higher Thresh", &flock_one.higherThresh, "min=0.025 max=1.0 step=0.025 keyIncr=h keyDecr=H" );
//	mParams.addSeparator();
//	mParams.addParam( "Attract Strength", &flock_one.attractStrength, "min=0.001 max=0.1 step=0.001 keyIncr=a keyDecr=A" );
//	mParams.addParam( "Repel Strength", &flock_one.repelStrength, "min=0.001 max=0.1 step=0.001 keyIncr=r keyDecr=R" );
//	mParams.addParam( "Orient Strength", &flock_one.orientStrength, "min=0.001 max=0.1 step=0.001 keyIncr=o keyDecr=O" );
//	mParams.addSeparator();
	mParams.addParam( "CV Threshhold", &silhouetteDetector->cvThresholdLevel, "min=0 max=255 step=1 keyIncr=t keyDecr=T" );
	
	//setup transformation from camera space to opengl world space
	imageToScreenMap.setToIdentity();
	imageToScreenMap.translate(Vec3f(getWindowSize().x/2, getWindowSize().y/2, 0));	//translate over and down
	imageToScreenMap.scale(Vec3f(-1*getWindowSize().x/320.0f, -1*getWindowSize().y/240.0f,1.0f));	//scale up
	polygons = new vector<Vec2i_ptr_vec>();
	
	currentBoidRuleNumber = 0;
	
	// ** stuff to create boid rulesets ** //
	
	// State 1: stuckOnYou - both flocks are attracted to the silhouette
	BoidSysPair stuckOnYou;
	stuckOnYou.flockOneProps.zoneRadius			= 80.0f;
	stuckOnYou.flockOneProps.lowerThresh		= 0.5f;
	stuckOnYou.flockOneProps.higherThresh		= 0.8f;
	stuckOnYou.flockOneProps.attractStrength	= 0.004f;
	stuckOnYou.flockOneProps.repelStrength		= 0.01f;
	stuckOnYou.flockOneProps.orientStrength		= 0.01f;
	stuckOnYou.flockOneProps.silThresh			= 1000.0f;
	stuckOnYou.flockOneProps.silRepelStrength	= -0.50f;
	stuckOnYou.flockOneProps.gravity			= false;
	stuckOnYou.flockOneProps.baseColor			= ColorA( CM_RGB, 0.784, 0.0, 0.714, 1.0);
	
	stuckOnYou.flockTwoProps=stuckOnYou.flockOneProps;
	stuckOnYou.imageColor						= ColorA( 0.08f, 0.0f, 0.1f, 1.0f);
	boidRulesets.push_back(stuckOnYou);
	
	// State 2: repel - both flocks are repeled by the silhouette
	BoidSysPair repel;
	repel.flockOneProps.zoneRadius				= 80.0f;
	repel.flockOneProps.lowerThresh				= 0.5f;
	repel.flockOneProps.higherThresh			= 0.8f;
	repel.flockOneProps.attractStrength			= 0.004f;
	repel.flockOneProps.repelStrength			= 0.01f;
	repel.flockOneProps.orientStrength			= 0.01f;
	repel.flockOneProps.silThresh				= 500.0f;
	repel.flockOneProps.silRepelStrength		= 1.00f;
	repel.flockOneProps.gravity					= false;
	repel.flockOneProps.baseColor				= ColorA( CM_RGB, 0.157, 1.0, 0.0,1.0);

	repel.flockTwoProps=repel.flockOneProps;
	repel.imageColor							= ColorA( 0.08f, 0.0f, 0.1f, 1.0f);	
	boidRulesets.push_back(repel);
	
	
	//// State 3: grav - like repel, but with gravity added
	BoidSysPair grav;
	grav.flockOneProps=repel.flockOneProps;
	grav.flockOneProps.gravity					= true;
	grav.flockOneProps.repelStrength			= 0.005f;
	grav.flockOneProps.baseColor				= ColorA( CM_RGB, 0.157, 0.0, 1.0,1.0);
	grav.flockTwoProps=grav.flockOneProps;
	grav.imageColor								= ColorA( 0.08f, 0.0f, 0.1f, 1.0f);
	boidRulesets.push_back(grav);
	
	
	//// State 4: diff - one flock is attracted to the silhouette and the other is repeled
	BoidSysPair diff;
	
	diff.flockOneProps=repel.flockOneProps;
	diff.flockTwoProps=stuckOnYou.flockOneProps;
	diff.imageColor								= ColorA( 0.08f, 0.0f, 0.1f, 1.0f);
	boidRulesets.push_back(diff);
	 
	
}

void BoidsApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'p' ){
		if (newFlock%2) {
			flock_one.addBoids( NUM_PARTICLES_TO_SPAWN );
			newFlock++;
		}else {
			flock_two.addBoids( NUM_PARTICLES_TO_SPAWN );
		}

	} else if( event.getChar() == ' ' ){
		mSaveFrames = !mSaveFrames;
	}
}


void BoidsApp::update()
{	
	
	double deltaT = lastFrameTime - getElapsedSeconds();
	
	//silly variable names, but let's hope it works
	if(isFullScreen() != shouldBeFullscreen) {
		setFullScreen(shouldBeFullscreen);
	}
	
	mEye	= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
	gl::rotate( mSceneRotation);
	
	
	// image CODE
	
	//gl::enableAlphaBlending();
	//	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	
	//gl::enableAdditiveBlending();
	//glEnable(GL_BLEND);
	//glEnable( GL_TEXTURE_2D );
	
	//gl::enableDepthWrite( true );
	//gl::enableDepthWrite( true );
	//glBlendFunc( GL_ONE, GL_SRC_ALPHA );

	
	if (checkTime()) {
		//get the next boidRuleset.
		int boidRuleToUse = (currentBoidRuleNumber++ % boidRulesets.size());
		BoidSysPair thisPair = boidRulesets[boidRuleToUse];
		flock_one.zoneRadius		= thisPair.flockOneProps.zoneRadius;
		flock_one.lowerThresh		= thisPair.flockOneProps.lowerThresh;
		flock_one.higherThresh		= thisPair.flockOneProps.higherThresh;
		flock_one.attractStrength	= thisPair.flockOneProps.attractStrength;
		flock_one.repelStrength		= thisPair.flockOneProps.repelStrength;
		flock_one.orientStrength	= thisPair.flockOneProps.orientStrength;
		flock_one.silThresh			= thisPair.flockOneProps.silThresh;
		flock_one.silRepelStrength	= thisPair.flockOneProps.silRepelStrength;
		flock_one.gravity			= thisPair.flockOneProps.gravity;
		flock_one.setColor(thisPair.flockOneProps.baseColor);

		
		
		flock_two.zoneRadius		= thisPair.flockTwoProps.zoneRadius;
		flock_two.lowerThresh		= thisPair.flockTwoProps.lowerThresh;
		flock_two.higherThresh		= thisPair.flockTwoProps.higherThresh;
		flock_two.attractStrength	= thisPair.flockTwoProps.attractStrength;
		flock_two.repelStrength		= thisPair.flockTwoProps.repelStrength;
		flock_two.orientStrength	= thisPair.flockTwoProps.orientStrength;
		flock_two.silThresh			= thisPair.flockTwoProps.silThresh;
		flock_two.silRepelStrength	= thisPair.flockTwoProps.silRepelStrength;
		flock_two.gravity			= thisPair.flockTwoProps.gravity;
		flock_two.setColor(thisPair.flockTwoProps.baseColor);	
		
		imageColor					= thisPair.imageColor;
	 }
	 
	
	
	//OpenCV IO
	//Only do OpenCV business if capture device is open and a new frame is ready
	if( capture && capture.checkNewFrame() ) {		
		polygons->clear();
		ci::Surface captureSurface = capture.getSurface();
		ci::Surface outputSurface = captureSurface;
		silhouetteDetector->processSurface(&captureSurface,polygons,&outputSurface);	//this only works because processSurface doesn't retain either pointer
		
		texture = outputSurface;
		flock_one.applySilhouetteToBoids(polygons,&imageToScreenMap);
		flock_two.applySilhouetteToBoids(polygons,&imageToScreenMap);
	}	 
	
	flock_one.applyForceToBoids();
	if( flock_one.centralGravity ) flock_one.pullToCenter( mCenter );
	flock_one.update(deltaT,getElapsedSeconds());
	
	flock_two.applyForceToBoids();
	if( flock_two.centralGravity) flock_two.pullToCenter( mCenter);
	flock_two.update(deltaT,getElapsedSeconds());
	
}

// Mouse Code ///

void BoidsApp::mouseDown( MouseEvent event )
{
	flock_one.mMousePressed = true;
	flock_two.mMousePressed = true;
	
	BoidsApp::updateMousePosition(event);
}

void BoidsApp::mouseUp( MouseEvent event )
{
	flock_one.mMousePressed = false;
	flock_two.mMousePressed = false;
}

void BoidsApp::mouseDrag( MouseEvent event )
{
	updateMousePosition(event);
}

//NOTE: The mouse position is based on a camera viewing from the initialized orientation only. If you rotate the view of the world, the mapping fails...
void BoidsApp::updateMousePosition(MouseEvent event){
	//flock_one.mousePos = Vec3f(-1*((event.getPos().x)-(getWindowSize().x/2)), -1*((getWindowSize().y/2)-(event.getPos().y)), 0.0f);
//	flock_two.mousePos = Vec3f(-1*((event.getPos().x)-(getWindowSize().x/2)), -1*((getWindowSize().y/2)-(event.getPos().y)), 0.0f);
	flock_one.mousePos = Vec3f(((event.getPos().x)-(getWindowSize().x/2)), ((getWindowSize().y/2)-(event.getPos().y)), 0.0f);
	flock_two.mousePos = Vec3f(((event.getPos().x)-(getWindowSize().x/2)), ((getWindowSize().y/2)-(event.getPos().y)), 0.0f);
}

// END MOUSE CODE//

void BoidsApp::draw()
{	
	
	glEnable( GL_TEXTURE_2D );
	gl::clear( Color( 0, 0, 0 ), true );	//this clears the old images off the window.
	
	
	mParticleTexture.bind();
	flock_one.draw();
	flock_two.draw();
	mParticleTexture.unbind();
	
	//drawCapture();
	drawPolyLines();

	/*
	if( mSaveFrames ){
		writeImage( getHomeDirectory() + "flocking/image_" + toString( getElapsedFrames() ) + ".png", copyWindowSurface() );
	}
		*/
	
	

	
	// DRAW PARAMS WINDOW
	params::InterfaceGl::draw();
}


void BoidsApp::drawPolyLines(){
	//DRAW THE POLYGONS
	//glPushMatrix();
	
	gl::enableDepthRead( true );
	gl::enableDepthWrite( true );
	
	gl::pushModelView();
	gl::multModelView(imageToScreenMap);
	glTranslatef(0.0f,0.0f,-0.01f);
	//glTranslatef(-1*getWindowSize().x/2,  -1*getWindowSize().y/2, 0.1f);
	//glScalef(getWindowSize().x/320.0f, getWindowSize().y/240.0f,1.0);	//scale up to fill the screen, same as for the video image

	glColor3f(0.3f,0.35f,0.3f);
	glLineWidth(4.5f);
	for(vector<Vec2i_ptr_vec>::iterator polygon = polygons->begin(); polygon!=polygons->end();++polygon) {
		glBegin(GL_LINE_STRIP);
		for(vector<Vec2i_ptr>::iterator point = polygon->get()->begin(); point!=polygon->get()->end();++point) {
			glVertex2f(point->get()->x,point->get()->y);
		}
		glEnd();
	}
	gl::popModelView();
	}

void BoidsApp::drawCapture(){
	if( texture){
		glPushAttrib(GL_CURRENT_BIT);
		

		
		gl::enableDepthRead( false );
		gl::enableDepthWrite( true );
		
		glDepthMask( GL_FALSE ); //IMPORTANT
		glDisable( GL_DEPTH_TEST ); //IMPORTANT
		glEnable( GL_BLEND ); //IMPORTANT
		glBlendFunc( GL_SRC_ALPHA, GL_ONE ); //IMPORTANT
		
		texture.bind();
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, 0.0f ) );
		gl::color(imageColor);
		gl::pushModelView();
		gl::multModelView(imageToScreenMap);
		gl::draw(texture);
		gl::popModelView();
		texture.unbind();
		
		glPopAttrib();
		
	}
}


bool BoidsApp::checkTime()
{
	time_t newTime = time(&newTime);
	double dif;
	
	dif = difftime(newTime,lastChange);
	
	if ( dif >= changeInterval) {
		lastChange = newTime;
		return TRUE;
	} else {
		return FALSE;
	}
}

CINDER_APP_BASIC( BoidsApp, RendererGl )
