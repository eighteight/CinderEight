//
//  ofxARDroneCommand.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 06/12/2012.
//  www.memo.tv
//
//

#include "ofxARDroneCommand.h"
#include "cinder/Utilities.h"
namespace ofxARDrone {
    
#define MAX_COMMAND_LENGTH      1024
    
    //--------------------------------------------------------------
    void Command::setName(string commandName) {
        this->commandName = commandName;
    }
    
    //--------------------------------------------------------------
    void Command::addInt(uint32_t i) {
        params.push_back(ci::toString(i));
    }
    
    //--------------------------------------------------------------
    void Command::addFloat(float f) {
        params.push_back(ci::toString(*(uint32_t*)(&f)));
    }

    //--------------------------------------------------------------
    void Command::addString(string s) {
        params.push_back("\""+s+"\"");
    }
    
    //--------------------------------------------------------------
    void Command::reset() {
        params.clear();
    }
    
    //--------------------------------------------------------------
    string Command::getString(uint32_t sequenceNumber) {
        string s = "AT*";
        s += commandName;
        s += "=";
        s +=  ci::toString(sequenceNumber);
        for(int i=0; i<params.size(); i++) {
            s += ",";
            s += params[i];
        }
//        s += char(0x0A);
        
        if(s.length()>=MAX_COMMAND_LENGTH) std::cerr<<"Drone::makeATCommand - length of ATCommand too long " + ci::toString(s.length());
        return s;
    }

}