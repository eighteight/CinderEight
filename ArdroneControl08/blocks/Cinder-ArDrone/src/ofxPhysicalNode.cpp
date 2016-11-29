//
//  ofxPhysicalNode.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 08/12/2012.
//
//

#include "ofxPhysicalNode.h"


//--------------------------------------------------------------
ofxPhysicalNode::ofxPhysicalNode() {
    lastUpdateMillis = 0;
}

//--------------------------------------------------------------
void ofxPhysicalNode::resetPhysics() {
    resetTransform();
    posVel.set(0, 0, 0);
    rotVel.set(0, 0, 0);
}


//--------------------------------------------------------------
void ofxPhysicalNode::updatePhysics() {
//    float nowMillis = ofGetElapsedTimeMillis();
    float deltaTime = 0.0f;//(nowMillis - lastUpdateMillis)/1000.0f;
//    lastUpdateMillis = ofGetElapsedTimeMillis();
    
    cinder::Vec3f pos(getPosition());
    if(maxPosVel.x > 0) posVel.x = cinder::math<float>::clamp(posVel.x, -maxPosVel.x, maxPosVel.x);
    if(maxPosVel.y > 0) posVel.y = cinder::math<float>::clamp(posVel.y, -maxPosVel.y, maxPosVel.y);
    if(maxPosVel.z > 0) posVel.z = cinder::math<float>::clamp(posVel.z, -maxPosVel.z, maxPosVel.z);
    pos += posVel * deltaTime;
    setPosition(pos);
    posVel -= posVel * posDrag; // TODO: how to make this fps independent?
    if(posVel.lengthSquared() < 0.00001) posVel.set(0, 0, 0);
    
    cinder::Vec3f rot(getOrientationEuler());
    if(maxRotVel.x > 0) rotVel.x = cinder::math<float>::clamp(rotVel.x, -maxRotVel.x, maxRotVel.x);
    if(maxRotVel.y > 0) rotVel.y = cinder::math<float>::clamp(rotVel.y, -maxRotVel.y, maxRotVel.y);
    if(maxRotVel.z > 0) rotVel.z = cinder::math<float>::clamp(rotVel.z, -maxRotVel.z, maxRotVel.z);
    rot += rotVel * deltaTime;
    setOrientation(rot);
    rotVel -= rotVel * rotDrag; // TODO: how to make this fps independent?
    if(rotVel.lengthSquared() < 0.00001) rotVel.set(0, 0, 0);
}