//
//  ofxARDroneState.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 07/12/2012.
//  www.memo.tv
//
//

#include "ofxARDroneState.h"
#include "cinder/Utilities.h"

using namespace cinder;
namespace ofxARDrone {
    
    //--------------------------------------------------------------
    State::State() {
        memset(this, 0, sizeof(*this));
    }
    
    //--------------------------------------------------------------
    //    State::~State() {
    //
    //    }
    
    
    //--------------------------------------------------------------
    void State::setConnected(bool b, int revertMillis){
        std::cout<<"ofxARDrone::State::setConnected - " + toString(b) + ", " + toString(revertMillis);
        bConnected.set(b, revertMillis, true);
    }
    
    //--------------------------------------------------------------
    void State::setTakingOff(bool b, int revertMillis) {
        std::cout<<"ofxARDrone::State::setTakingOff - " + toString(b) + ", " + toString(revertMillis);
        
        bTakingOff.set(b, revertMillis);
        
        if(bTakingOff.get()) {
            setLanding(false, 0);
        }
    }
    
    
    //--------------------------------------------------------------
    void State::setLanding(bool b, int revertMillis) {
        std::cout<<"ofxARDrone::State::setLanding - " + toString(b) + ", " + toString(revertMillis);
        
        bLanding.set(b, revertMillis);
        
        if(bLanding.get()) setTakingOff(false, 0);
    }
    
    //--------------------------------------------------------------
    void State::setCalibratingHorizontal(bool b, int revertMillis) {
        std::cout<<"ofxARDrone::State::setCalibratingHorizontal - " + toString(b) + ", " + toString(revertMillis);
        bCalibratingHorizontal.set(b, revertMillis);
    }
    
    //--------------------------------------------------------------
    void State::setCalibratingMagnetometer(bool b, int revertMillis) {
        std::cout<<"ofxARDrone::State::setCalibratingMagnetometer - " + toString(b) + ", " + toString(revertMillis);
        bCalibratingMagnetometer.set(b, revertMillis);
    }
    
    //--------------------------------------------------------------
    void State::update() {
        bConnected.update();
        bTakingOff.update();
        bLanding.update();
        bCalibratingHorizontal.update();
        bCalibratingMagnetometer.update();
    }
    
    
    
    //--------------------------------------------------------------
    //--------------------------------------------------------------
    //--------------------------------------------------------------
    // if revertMillis is nonzero, automatically cancels state after this many milliseconds
    void State::Parameter::set(bool b, int revertMillis, bool resetTimer) {
        if(value != b || resetTimer) {
            value = b;
            changeTimestamp = cinder::app::getElapsedSeconds();// ofGetElapsedTimeMillis();
            revertTimestamp = revertMillis > 0 ? changeTimestamp + revertMillis : 0;
        }
    }
    
    //--------------------------------------------------------------
    bool State::Parameter::get() {
        return value;
    }
    
    //--------------------------------------------------------------
    int State::Parameter::millisSinceLastChange() {
        return cinder::app::getElapsedSeconds() - changeTimestamp;
    }
    
    //--------------------------------------------------------------
    void State::Parameter::update() {
        if(value && revertTimestamp > 0 && cinder::app::getElapsedSeconds() > revertTimestamp) {
            set(false, 0);
        }
    }
    

}