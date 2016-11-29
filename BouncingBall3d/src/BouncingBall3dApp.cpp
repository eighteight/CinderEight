#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Timer.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "FmodexPlayer.h"

#define ROOMDEPTH -1000
using namespace ci;
using namespace ci::app;
using namespace std;

typedef shared_ptr<class Ball> BallRef;

class Ball {
public:
	Ball();

	void setup();
	void update();
	void draw( const gl::VboMesh &mesh, bool useMotionBlur = true );
public:
	static const int RADIUS = 20;
private:
	float	mGravity;
	FmodexPlayer floorPlayer, wallPlayer;
	Vec3f	mPrevPosition;
	Vec3f	mPosition;
	Vec3f	mVelocity;
};

Ball::Ball()
{
	setup();
}

void Ball::setup()
{
	mGravity = 1.0f;

	mPosition = Vec3f( 0.5f * getWindowWidth(), 0.2f * getWindowHeight(), 0.5*ROOMDEPTH );
	mPrevPosition = mPosition;

	float x = Rand::randFloat(-15.0f, 15.0f);
	float y = Rand::randFloat(-25.0f,  0.0f);
    float z = Rand::randFloat(-25.0f,  25.0f);
	mVelocity = Vec3f(1, 0, -25.0f);
    
    
    floorPlayer.loadSound(App::getResourcePath("22769__franciscopadilla__66-low-timbale.wav").string());
	floorPlayer.setVolume(0.75f);
	floorPlayer.setLoop(false);
    floorPlayer.setMultiPlay(true);
    
    wallPlayer.loadSound(App::getResourcePath("22769__franciscopadilla__66-low-timbale.wav").string());
	wallPlayer.setVolume(0.25f);
	wallPlayer.setLoop(false);
    wallPlayer.setMultiPlay(true);
    
}

void Ball::update()
{
	// store current position
	mPrevPosition = mPosition;

	// first, update the ball's velocity
	mVelocity.y += mGravity;

	// next, update the ball's position
	mPosition += mVelocity;

	// finally, perform collision detection:
	//	1) check if the ball hits the left or right side of the window
	if( mPosition.x < (0.0f + RADIUS) || mPosition.x > (getWindowWidth() - RADIUS) ) {
		// to reduce the visual effect of the ball missing the border, 
		// set the previous position to where we are now
		mPrevPosition = mPosition;
		// move the ball back into window without adding energy,
		// by placing it where it would have been without friction
		mPosition.x -= mVelocity.x;
		// reduce velocity due to friction
		mVelocity.x *= -0.95f;
        wallPlayer.play();
	}
    // 1a check if the ball hits the front or rear of the room
    if( mPosition.z > (0.0f + RADIUS) || mPosition.z < (ROOMDEPTH - RADIUS) ) {
		// to reduce the visual effect of the ball missing the border, 
		// set the previous position to where we are now
		mPrevPosition = mPosition;
		// move the ball back into window without adding energy,
		// by placing it where it would have been without friction
		mPosition.z -= mVelocity.z;
		// reduce velocity due to friction
		mVelocity.z *= -0.95f;
        wallPlayer.setVolume(abs(mVelocity.z)/15);
        wallPlayer.play();
	}
	//	2) check if the ball this the bottom of the window
	if( mPosition.y > (getWindowHeight() - RADIUS) ) {		
		// to reduce the visual effect of the ball missing the border, 
		// set the previous position to where we are now
		mPrevPosition = mPosition;
		// move the ball back into window without adding energy,
		// by placing it where it would have been without friction
		mPosition.y -= mVelocity.y;
		// reduce velocity due to friction
		mVelocity.x *=  0.99f;
		mVelocity.y *= -0.975f;
        floorPlayer.setSpeed(abs(mVelocity.y)/15);
        floorPlayer.play();
	}
}

void Ball::draw( const gl::VboMesh &mesh, bool useMotionBlur )
{
	static const int trailsize = 2;

	gl::pushModelView();

	// draw ball with motion blur (using addative blending)
	if(useMotionBlur) {
		gl::color( Color::white() / trailsize );

		Vec3f v(0.0f, 0.0f, 0.0f);
		for(size_t i=0;i<trailsize;++i) {
			Vec3f d = mPrevPosition.lerp( i / (float) (trailsize - 1), mPosition ) - v;
			v += d;

			gl::translate( d );
			gl::draw( mesh );
		}
	}
	else {
		gl::color( Color::white() );
		gl::translate( mPosition );
		gl::draw( mesh );
	}
		
	gl::popModelView();
}

class BouncingBall3dApp : public AppBasic {
public:
	void setup();
	void update();
	void draw();

	void keyDown( KeyEvent event );	
private:
	void step();
private:
	bool		mUseMotionBlur;

	// simulation timer
	uint32_t	mStepsPerSecond;
	uint32_t	mStepsPerformed;
	Timer		mTimer;

	// our list of balls 
	std::vector<BallRef> mBalls;

	// mesh and texture
	gl::VboMesh	mMesh;
	gl::Texture mTexture;
};

void BouncingBall3dApp::setup()
{
	mUseMotionBlur = true;

	// set some kind of sensible maximum to the frame rate
	setFrameRate(200.0f);

	// initialize simulator
	mStepsPerSecond = 60;
	mStepsPerformed = 0;

	// create a single ball
	mBalls.push_back( BallRef( new Ball() ) );

	// create ball mesh ( much faster than using gl::drawSolidCircle() )
	size_t slices = 30;

	std::vector<Vec3f> positions;
	std::vector<Vec2f> texcoords;
	std::vector<uint32_t> indices;

	indices.push_back( positions.size() );
	texcoords.push_back( Vec2f(0.5f, 0.5f) );
	positions.push_back( Vec3f::zero() );

	for(size_t i=0;i<=slices;++i) {	
		float angle = i / (float) slices * 2.0f * (float) M_PI;
		Vec2f v(sinf(angle), cosf(angle));

		indices.push_back( positions.size() );
		texcoords.push_back( Vec2f(0.5f, 0.5f) + 0.5f * v );
		positions.push_back( Ball::RADIUS * Vec3f(v, 0.0f) );
	}

	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticIndices();

	mMesh = gl::VboMesh( (size_t) (slices + 2), (size_t) (slices + 2), layout, GL_TRIANGLE_FAN );
	mMesh.bufferPositions( &positions.front(), positions.size() );
	mMesh.bufferTexCoords2d(0, texcoords);
	mMesh.bufferIndices( indices );

	// load texture
	mTexture = gl::Texture( loadImage( loadAsset("ball.png") ) );

	// start simulation
	mTimer.start();
}

void BouncingBall3dApp::update()
{
	// determine how many simulation steps should have been performed until now
	uint32_t stepsTotal = static_cast<uint32_t>( floor( mTimer.getSeconds() * mStepsPerSecond ) );

	// perform the remaining steps
	std::vector<BallRef>::iterator itr;
	while( mStepsPerformed < stepsTotal ) {
		for(itr=mBalls.begin();itr!=mBalls.end();++itr)
			(*itr)->update();

		mStepsPerformed++;
	}
}

void BouncingBall3dApp::draw()
{
	gl::clear(); 
	gl::enableAdditiveBlending();

	if(mTexture) mTexture.enableAndBind();
	
	std::vector<BallRef>::iterator itr;
	for(itr=mBalls.begin();itr!=mBalls.end();++itr)
		(*itr)->draw( mMesh, mUseMotionBlur );

	if(mTexture) mTexture.unbind();
		
	gl::disableAlphaBlending();
}

void BouncingBall3dApp::keyDown( KeyEvent event )
{
	std::vector<BallRef>::iterator itr;

	switch( event.getCode() )
	{
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;
	case KeyEvent::KEY_SPACE:
		// reset all balls
		for(itr=mBalls.begin();itr!=mBalls.end();++itr)
			(*itr)->setup();
		break;
	case KeyEvent::KEY_RETURN:
		// pause/resume simulation
		if(mTimer.isStopped()) {
			mStepsPerformed = 0;
			mTimer.start();
		}
		else mTimer.stop();
		break;
	case KeyEvent::KEY_PLUS:
	case KeyEvent::KEY_KP_PLUS:
		// create a new ball
		mBalls.push_back( BallRef( new Ball() ) );
		break;
	case KeyEvent::KEY_MINUS:
	case KeyEvent::KEY_KP_MINUS:
		// remove the oldest ball
		if(!mBalls.empty())
			mBalls.erase( mBalls.begin() );
		break;
	case KeyEvent::KEY_v:
		gl::enableVerticalSync( !gl::isVerticalSyncEnabled() );
		break;
	case KeyEvent::KEY_m:
		mUseMotionBlur = !mUseMotionBlur;
		break;
	}
}

CINDER_APP_BASIC( BouncingBall3dApp, RendererGl )
