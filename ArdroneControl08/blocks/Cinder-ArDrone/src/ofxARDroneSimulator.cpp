//
//  ofxARDroneSimulator.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 07/12/2012.
//  www.memo.tv
//
//


#include "ofxARDroneSimulator.h"

#include "ofxARDrone.h"
#include "cinder/gl/gl.h"

using namespace cinder;
namespace ofxARDrone {
    
    //--------------------------------------------------------------
    Simulator::Simulator() {
        std::cout<<"ofxARDrone::Simulator::Simulator";
        drone = NULL;
        
        rollRotMult = -10;
        pitchRotMult = 10;
        rollVelMult = -10;
        pitchVelMult = 10;
        liftVelMult = 10;
        spinVelMult = -5;
        
        // physics parameters
        posDrag.set(0.05, 0.1, 0.05);
        rotDrag.set(0.01, 0.04, 0.01);
    }
    
    //--------------------------------------------------------------
    Simulator::~Simulator() {
        std::cout<<"ofxARDrone::Simulator::~Simulator";
    }
    
    //--------------------------------------------------------------
    void Simulator::setup(Drone *drone) {
        std::cout<<"ofxARDrone::Simulator::setup -  + toString(drone)";
        this->drone = drone;
    }

    //--------------------------------------------------------------
    void Simulator::reset() {
        cout<<"resetTransform()";
        resetPhysics();
    }
    
    //--------------------------------------------------------------
    void Simulator::update() {
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        
        posVel += -drone->controller.rollAmount * rollVelMult * getXAxis();
        posVel += drone->controller.liftSpeed * liftVelMult * getYAxis();
        posVel += drone->controller.pitchAmount * pitchVelMult * getZAxis();
  
        //Vec3f rot(getOrientationEuler());
        rotVel.y += drone->controller.spinSpeed * spinVelMult;
        // TODO: do pitch and tilt as well
        
        updatePhysics();
    }
    
    //--------------------------------------------------------------
    void Simulator::customDraw() {
        // all units in cm
        float bladeLength = 20;
    
        // draw body
        cinder::gl::color( Color( 1, 0, 0) );
        cinder::gl::pushMatrices();
        gl::scale(5, 5, 15);
        gl::drawCube(Vec3f(0.,0.,0.), Vec3f(1.0,1.0,1.0));
        gl::popMatrices();
        
        // draw head
        gl::color( Color( 0, 1, 0) );

        gl::drawCube(Vec3f(0., 0., -10.), Vec3f(5.0,5.0,5.0));
        
        // draw cross frames
        gl::color( Color( 1, 1, 1) );

        for(int i=0; i<2; i++) {
            glPushMatrix();
            gl::rotate(45 + i * 90);
            gl::scale(Vec3f(bladeLength * 1.4f, 1, 1));
            gl::drawCube(Vec3f(0.,3.,0.), Vec3f(1.0,1.0,1.0));
            gl::popMatrices();
        }
        
        
        // draw blades
        cinder::gl::color( ColorA( 1, 0, 0, 1 ) );
        for(int i=0; i<2; i++) {
            for(int j=0; j<2; j++) {
                glPushMatrix();
                float x = (i-0.5) * bladeLength;
                float z = (j-0.5) * bladeLength;
                gl::translate(Vec3f(x, 3, z));

                float rot = i * 73 + j * 130;
                if(drone->state.isFlying() || drone->state.isTakingOff()) {
                    int sign = (i+j)%2 ? 1 : -1;
                    rot +=  0;//ofGetElapsedTimeMillis() * 100;
                    rot *= sign;
                }
                gl::rotate(rot);
                gl::scale(Vec3f(20, 1, 2));
                gl::drawCube(Vec3f(0.,0.,0.), Vec3f(1.0,1.0,1.0));
                gl::popMatrices();
            }
        }
        
//        ofDrawAxis(30);

    }
    
};