/*
 *  Boid.h
 *  Boids
 *
 *  Created by Ryan Spicer on 11/9/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include <vector>

//all design is compromise.
class BoidController;

class Boid {
public:
	Boid();
	Boid( ci::Vec3f pos, ci::Vec3f vel, bool followed, BoidController *parent );
	void pullToCenter( const ci::Vec3f &center );
	void update( bool flatten);
	void draw();
	void limitSpeed();
	void addNeighborPos( ci::Vec3f pos );
	
	ci::Vec3f	pos;
	ci::Vec3f	tailPos;
	ci::Vec3f	vel;
	ci::Vec3f	velNormal;
	ci::Vec3f	acc;
	
	//debug
	ci::Vec3f	closestSilhouettePoint;
	
	ci::Vec3f	mNeighborPos;
	int			mNumNeighbors;
	
	ci::Color	mColor;
	
	float		mDecay;
	float		mRadius;
	float		mLength;
	float		mMaxSpeed, mMaxSpeedSqrd;
	float		mMinSpeed, mMinSpeedSqrd;
	float		mFear;
	float		mCrowdFactor;
	
	bool		mIsDead;
	bool		mFollowed;
	bool		mGravity;
	bool		drawClosestSilhouettePoint;
	
	
	// ** trail code ** //
	int			mLen;
	float		mInvLen;
	std::vector<ci::Vec3f> mLoc;
	
	void renderQuadStripTrail();
	
	float		radius;
	// ** end trail code ** //
	
	BoidController *parent;
};