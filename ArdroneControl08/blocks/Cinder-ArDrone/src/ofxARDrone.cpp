//
//  ofxARDrone.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 06/12/2012.
//  www.memo.tv
//
//

#include "ofxARDrone.h"


namespace ofxARDrone {
    
    Drone::Drone() {
        std::cout<<"ofxARDrone::Drone::Drone";
        commInfo.hostip = "192.168.1.1";
        commInfo.commandPort = 5556;
        commInfo.navDataPort = 5554;
        commInfo.videoPort   = 5555;
        commInfo.controlPort = 5559;
    }
    
    //--------------------------------------------------------------
    Drone::~Drone() {
        std::cout<<"ofxARDrone::Drone::~Drone";
        if(state.isFlying()) controller.land(true, 0);
        disconnect();
    }
    
    
    
    //--------------------------------------------------------------
    void Drone::connect() {
        std::cout<<"ofxARDrone::Drone::connect";
        
        sequenceNumber = 1;

        controller.setup(this);
        dataReceiver.setup(this);
    }
    
    //--------------------------------------------------------------
    void Drone::disconnect() {
        std::cout<<"ofxARDrone::Drone::disconnect";
        
        controller.close();
        dataReceiver.close();
    }
    
    //--------------------------------------------------------------
    void Drone::update() {
        dataReceiver.update();
        controller.update();
        state.update();
    }

    //--------------------------------------------------------------
    uint32_t Drone::getSequenceNumber() {
        return sequenceNumber;
    }
    
    //--------------------------------------------------------------
    void Drone::resetSequenceNumber() {
        sequenceNumber = 0;
    }
    
    
}