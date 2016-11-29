//
//  Balloon.cpp
//  BigBang
//
//  Created by Robert Hodgin on 5/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Sphere.h"
#include "Balloon.h"

#define SPRING_STRENGTH 0.05
#define SPRING_DAMPING 0.25
#define REST_LENGTH 0.0

using namespace ci;
using namespace ci::app;

Balloon::Balloon(){}

Balloon::Balloon( const Vec3f &pos, int presetIndex ): mPos( pos )
{
    mGravity = -1.0f;
	mVel		= Rand::randVec3f() * Rand::randFloat( 3.0f, 13.0f );
	mAcc		= Vec3f::zero();

	mSpringPos	= mPos - Vec3f( 0.0f, 20.0f, 0.0f );
	mSpringVel	= mVel;
	mSpringPos	= mAcc;
	
	mRadius		= Rand::randFloat( 22.0f, 18.0f );
	mMatrix.setToIdentity();
	
	if( presetIndex == 4 ){
		if( Rand::randBool() ){
			mColor = Color( 0.5f, 0.5f, 0.5f );
		} else {
			mColor = Color::black();
		}		
	} else {
		int i = Rand::randInt(4);
		if( i == 0 ){
			mColor = Color( 0.6f, 0.1f, 0.0f );
		} else if( i == 1 ){
			mColor = Color( 1.0f, 0.5f, 0.0f );
		} else if( i == 2 ){
			mColor = Color( 0.0f, 0.0f, 0.0f );
		}
	}
	
	mBuoyancy	= Rand::randFloat( 0.01f, 0.07f );
	
	mAge		= 0.0f;
	mLifespan	= 10000.0f;
	mIsDead		= false;
    
    ////////
    float x = Rand::randFloat(-15.0f, 15.0f);
	float y = Rand::randFloat(-25.0f,  0.0f);
    float z = Rand::randFloat(-25.0f,  25.0f);
	mVel = Vec3f(x, y, z);
}

void Balloon::setup(){
    std::cout<<App::getResourcePath("timbale.wav").string()<<std::endl;
floorPlayer.loadSound(App::getResourcePath("timbale.wav").string());
floorPlayer.setVolume(0.75f);
floorPlayer.setLoop(false);
floorPlayer.setMultiPlay(true);
    //    //floorPlayer.play();
    //    
wallPlayer.loadSound(App::getResourcePath("timbale.wav").string());
wallPlayer.setVolume(0.25f);
wallPlayer.setLoop(false);
wallPlayer.setMultiPlay(true);
    //    //wallPlayer.play();
}

void Balloon::updateSpring( float dt )
{
	ci::Vec3f dir		= mSpringPos - ( mPos - Vec3f( 0.0f, 20.0f, 0.0f ) );
	float dist			= dir.length();
	dir.safeNormalize();
	float springForce	= -( dist - REST_LENGTH ) * SPRING_STRENGTH;
	float dampingForce	= -SPRING_DAMPING * ( dir.x*mSpringVel.x + dir.y*mSpringVel.y + dir.z*mSpringVel.z );
	float r				= springForce + dampingForce;
	dir *= r;
	mSpringAcc += dir;
	
	mSpringVel += mSpringAcc * dt;
	mSpringPos += mSpringVel * dt;
	mSpringVel -= mSpringVel * 0.04 * dt;
	mSpringAcc = ci::Vec3f::zero();
}

void Balloon::update( const Camera &cam, const Vec3f &roomDims, float dt )
{
	///////updateSpring( dt );
	
//	mAcc += Vec3f( 0.0f, mBuoyancy, 0.0f );
//	mVel += mAcc * dt;
//	mPos += mVel * dt;
//	mVel -= mVel * 0.025f * dt;
//	mAcc = Vec3f::zero();
	
	mScreenPos		= cam.worldToScreen( mPos, app::getWindowWidth(), app::getWindowHeight() );
//	mDistToCam		= -cam.worldToEyeDepth( mPos );
	Sphere sphere	= Sphere( mPos, mRadius );
	mScreenRadius	= cam.getScreenRadius( sphere, app::getWindowWidth(), app::getWindowHeight() );
	
//	checkBounds( roomDims );
	
//	Vec3f tiltAxis = ( mPos - mSpringPos ).normalized();
	
//	mMatrix.setToIdentity();
//	mMatrix.translate( mPos );
//	mMatrix.rotate( tiltAxis, 2.0f );
//	mMatrix.rotate( Vec3f( mAge * mXRot, mYRot, mAge * mZRot ) );
//	mMatrix.scale( Vec3f( mRadius, mRadius, mRadius ) );
	
	mAge += dt;
    
    
    
    ////////////
    // store current position
	mPrevPosition = mPos;
    
	// first, update the ball's velocity
	mVel.y += mGravity;
    
	// next, update the ball's position
	mPos += mVel;
    	checkBounds( roomDims );
//	// finally, perform collision detection:
//	//	1) check if the ball hits the left or right side of the window
//	if( mPos.x < (0.0f + mRadius) || mPos.x > (roomDims.x - mRadius) ) {
//		// to reduce the visual effect of the ball missing the border, 
//		// set the previous position to where we are now
//		mPrevPosition = mPos;
//		// move the ball back into window without adding energy,
//		// by placing it where it would have been without friction
//		mPos.x -= mVel.x;
//		// reduce velocity due to friction
//		mVel.x *= -0.95f;
//        //wallPlayer.play();
//	}
//    // 1a check if the ball hits the front or rear of the room
//    if( mPos.z > (0.0f + mRadius) || mPos.z < (-1000 - mRadius) ) {
//		// to reduce the visual effect of the ball missing the border, 
//		// set the previous position to where we are now
//		mPrevPosition = mPos;
//		// move the ball back into window without adding energy,
//		// by placing it where it would have been without friction
//		mPos.z -= mVel.z;
//		// reduce velocity due to friction
//		mVel.z *= -0.95f;
//        //wallPlayer.setVolume(abs(mVel.z)/15);
//        //wallPlayer.play();
//	}
//	//	2) check if the ball this the bottom of the window
//	if( mPos.y > (roomDims.y - mRadius) ) {		
//		// to reduce the visual effect of the ball missing the border, 
//		// set the previous position to where we are now
//		mPrevPosition = mPos;
//		// move the ball back into window without adding energy,
//		// by placing it where it would have been without friction
//		mPos.y -= mVel.y;
//		// reduce velocity due to friction
//		mVel.x *=  0.99f;
//		mVel.y *= -0.975f;
//        //floorPlayer.setSpeed(abs(mVel.y)/15);
//        //floorPlayer.play();
//	}
    
    mMatrix.setToIdentity();
	mMatrix.translate( mPos );
	mMatrix.scale( Vec3f( mRadius, mRadius, mRadius ) );
}

void Balloon::checkBounds( const Vec3f &roomDims )
{	
	if( mPos.x - mRadius < -roomDims.x ){
		mPos.x = -roomDims.x + mRadius;
		mVel.x *= -0.915f;
        wallPlayer.play();
	} else if( mPos.x + mRadius > roomDims.x ){
		mPos.x = roomDims.x - mRadius;
		mVel.x *= -0.915f;
        wallPlayer.play();
	}

	if( mPos.y - mRadius < -roomDims.y ){
		mPos.y = -roomDims.y + mRadius;
		mVel.y *= -0.915f;
        floorPlayer.setSpeed(abs(mVel.y)/15);
        floorPlayer.play();
	} else if( mPos.y + mRadius > roomDims.y ){
		mPos.y = roomDims.y - mRadius;
		mVel.y *= -0.915f;
        floorPlayer.setSpeed(abs(mVel.y)/15);
        floorPlayer.play();
	}
	
	if( mPos.z - mRadius < -roomDims.z ){
		mPos.z = -roomDims.z + mRadius;
		mVel.z *= -0.915f;
        wallPlayer.play();
	} else if( mPos.z + mRadius > roomDims.z ){
		mPos.z = roomDims.z - mRadius;
		mVel.z *= -0.915f;
        wallPlayer.play();
	}
}

void Balloon::draw()
{
	gl::color( mColor );
	gl::drawSphere( mPos, mRadius );
}