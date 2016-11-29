//
//  ofxPhysicalNode.h
//  ofxARDrone Example
//
//  Created by Memo Akten on 08/12/2012.
//
//



#pragma once

//#include "ofMain.h"
#include "ofNode.h"

class ofxPhysicalNode : public ofNode {
public:

    ofxPhysicalNode();

    void resetPhysics();
    void updatePhysics();
    
protected:
    cinder::Vec3f posVel;
    cinder::Vec3f rotVel;
    
    cinder::Vec3f posDrag;
    cinder::Vec3f rotDrag;
    
    cinder::Vec3f maxPosVel;
    cinder::Vec3f maxRotVel;
    
    unsigned long lastUpdateMillis;
    
};