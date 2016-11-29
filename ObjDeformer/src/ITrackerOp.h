/*
 * IHandTracker.h
 *
 *  Created on: Apr 17, 2011
 *      Author: eight
 */
#pragma once
#ifndef ITRACKER_H_
#define ITRACKER_H_

#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Timer.h"

const float TWEEN_SPEED = 0.08f;
class ITrackerOp {
public:
	virtual ~ITrackerOp(){};
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual bool isTracking()= 0;
	virtual void keyDown(cinder::app:: KeyEvent event ){};
	virtual void mouseDown(cinder::app::MouseEvent event){};
	virtual void mouseDrag(cinder::app:: MouseEvent event){};
	virtual void mouseUp(cinder::app:: MouseEvent event){};
	virtual cinder::vec3 getShift(){return cinder::vec3(0,0,0);};
	virtual cinder::vec3 getTargetPosition() const = 0;

	virtual void setup(){};

	float easing;
protected:
	virtual double getElapsedSeconds(){return 1;};
	virtual void   resetElapsedSeconds(){};


	cinder::Timer mTimer;
};

#endif /* ITRACKER_H_ */
