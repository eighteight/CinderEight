//
//  BallSystem.h
//  KTWRoom
//
//  Created by Gusev, Vladimir on 7/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef KTWRoom_BallSystem_h
#define KTWRoom_BallSystem_h

#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Timer.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "Ball.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef shared_ptr<class Ball> BallRef;

class BallSystem {
public:
	void setup(int);
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
    int depth;
};

void BallSystem::setup(int depth)
{
    this->depth = depth;
    
	mUseMotionBlur = true;
    
	// set some kind of sensible maximum to the frame rate
	setFrameRate(200.0f);
    
	// initialize simulator
	mStepsPerSecond = 60;
	mStepsPerformed = 0;
    
	// create a single ball
    BallRef ref = BallRef(new Ball());
    ref->setup(depth);
	mBalls.push_back(ref);
    
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

void BallSystem::update()
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

void BallSystem::draw()
{

	gl::enableAdditiveBlending();
    
	if(mTexture) mTexture.enableAndBind();
	
	std::vector<BallRef>::iterator itr;
	for(itr=mBalls.begin();itr!=mBalls.end();++itr){
		(*itr)->draw( mMesh, mUseMotionBlur );
    }
    
    if(mTexture) mTexture.unbind();
    
	gl::disableAlphaBlending();
}

void BallSystem::keyDown( KeyEvent event )
{
	std::vector<BallRef>::iterator itr;
    
	switch( event.getCode() )
	{
        case KeyEvent::KEY_ESCAPE:
            // quit the application
            break;
        case KeyEvent::KEY_SPACE:
            // reset all balls
            for(itr=mBalls.begin();itr!=mBalls.end();++itr)
                (*itr)->setup(depth);
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


#endif
