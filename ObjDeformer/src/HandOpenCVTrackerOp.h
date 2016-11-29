/*
 * HandOpenCVTracker.h
 *
 *  Created on: Apr 17, 2011
 *      Author: eight
 */
#pragma once
#ifndef HANDOPENCVTRACKER_H_
#define HANDOPENCVTRACKER_H_

#include "ITrackerOp.h"

#include "cinder/gl/Texture.h"

#include "CinderFreenect.h"
#include "CinderOpenCv.h"


using namespace cinder;
using namespace ci;
using namespace std;

class HandOpenCVTracker: public ITrackerOp {
public:
	HandOpenCVTracker();
    ~HandOpenCVTracker();

	void   setup();
	void   update();
	bool isTracking();
	void   draw();
    vec3 getTargetPosition() const;

private:
	KinectRef mKinect;

	float mThreshold, mBlobMin, mBlobMax;
	float mKinectTilt;

	gl::TextureRef mColorTexture, mDepthTexture, mCvTexture;
	Surface mDepthSurface;
    bool tracking;
	vec3 mTargetPosition;

};

#endif /* HANDOPENCVTRACKER_H_ */
