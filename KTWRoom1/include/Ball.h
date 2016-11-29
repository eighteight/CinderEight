//
//  Ball.h
//  KTWRoom
//
//  Created by Gusev, Vladimir on 7/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef KTWRoom_Ball_h
#define KTWRoom_Ball_h
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Vector.h"
#include "FmodexPlayer.h"

#define ROOMDEPTH -1000

typedef std::shared_ptr<class Ball> BallRef;

class Ball {
public:
	Ball();
    
	void setup(int depth);
	void update();
	void draw( const cinder::gl::VboMesh &mesh, bool useMotionBlur = true );
    cinder::Vec3f	mPosition;
public:
	static const int RADIUS = 20;
private:
	float	mGravity;
	FmodexPlayer floorPlayer, wallPlayer;
    cinder::Vec3f	mPrevPosition;

	cinder::Vec3f	mVelocity;
    cinder::Matrix44f	mMatrix;
};


using namespace ci;
using namespace ci::app;
using namespace std;

Ball::Ball()
{
}

void Ball::setup(int depth)
{
    mMatrix.setToIdentity();
	mGravity = 1.0f;
    
	mPosition = Vec3f( 0.5f * app::getWindowWidth(), 0.2f * app::getWindowHeight(), 0.5*depth );
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
    
    mMatrix.setToIdentity();
	mMatrix.translate( mPosition );
	mMatrix.scale( Vec3f( RADIUS, RADIUS, RADIUS ) );
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
            
			gl::translate( d/10.0f );
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


#endif
