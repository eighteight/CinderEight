//
//  ofxARDroneController.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 06/12/2012.
//  www.memo.tv
//
//

#include "ofxARDroneController.h"
#include "ofxARDrone.h"
#include "cinder/CinderMath.h"

namespace ofxARDrone {
    
    
    
    //--------------------------------------------------------------
    Controller::Controller() {
        cout<<"ofxARDrone::Controller::Controller"<<endl;
        drone = NULL;
        commandHistory.setMaxLength(0);
        int millisOfLastSend = 0;
        int millisSinceLastSend = 0;
        
        rollAmount = 0;
        pitchAmount = 0;
        liftSpeed = 0;
        spinSpeed = 0;
    }
    
    //--------------------------------------------------------------
    Controller::~Controller() {
        cout<<"ofxARDrone::Controller::~Controller"<<endl;
        close();
    }
    
    //--------------------------------------------------------------
    void Controller::setup(Drone* drone) {
        std::cout<<"ofxARDrone::Controller::setup -  + tostring(drone)"<<std::endl;
        this->drone = drone;

        if(drone == NULL) { std::cout<<"   Drone is NULL"; return; }
            
//        udpSender.Create();
//        udpSender.Connect(drone->commInfo.hostip.c_str(), drone->commInfo.commandPort);
//        udpSender.SetNonBlocking(true);
    }
    
    //--------------------------------------------------------------
    void Controller::close() {
        std::cout<<"ofxARDrone::Controller::close"<<std::endl;
        //udpSender.Close();
    }

    
    //--------------------------------------------------------------
    void Controller::update() {
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        
        State &state = drone->state;
        
        if(state.isCalibratingHorizontal()) {
            if(state.isFlying()) {
                std::cerr<<"ofxARDrone::Controller::update - shouldn't calibrate horizontal while flying";
                state.setCalibratingHorizontal(false, 0);
            } else {
                Command command;
                command.setName("FTRIM");
                queueCommand(command);
            }
        }
        
        
        if(state.isCalibratingMagnetometer()) {
            if(!state.isFlying()) {
                std::cerr<<"ofxARDrone::Controller::update - should only calibrate magnetometer while flying";
                state.setCalibratingMagnetometer(false, 0);
            } else {
                Command command;
                command.setName("CALIB");
                queueCommand(command);
            }
        }
        
        if(state.isTakingOff() || state.isLanding()) {
            Command command;
            command.setName("REF");
            
            msa::BitField32 bitField;
            bitField.clearAll();
            bitField.setBit(18);
            bitField.setBit(20);
            bitField.setBit(22);
            bitField.setBit(24);
            bitField.setBit(28);
            bitField.setBit(9, state.isTakingOff()); // keep sending until navdata says it's taken off, or landed
            command.addInt(bitField);
            
            queueCommand(command);
        }
        
        rollAmount = cinder::math<float>::clamp(rollAmount, -1, 1);
        pitchAmount = cinder::math<float>::clamp(pitchAmount, -1, 1);
        liftSpeed = cinder::math<float>::clamp(liftSpeed, -1, 1);
        spinSpeed = cinder::math<float>::clamp(spinSpeed, -1, 1);
        
        if(state.isFlying()) {
            Command command;
            command.setName("PCMD");
            command.addInt(1);
            command.addFloat(rollAmount);
            command.addFloat(pitchAmount);
            command.addFloat(liftSpeed);
            command.addFloat(spinSpeed);
            
            queueCommand(command);
        }
        
        
        if(commandString.empty() && getMillisSinceLastSend() > 50) resetCommunicationWatchdog();

        
        if(commandString.empty() == false) {
//            udpSender.Send(commandString.c_str(), commandString.length());
            commandHistory.push_front(commandString);

//            millisOfLastSend = ofGetElapsedTimeMillis();
            this->commandString = "";
        }
    }
    
    //-------------------------------------------------------------- 
//    void Controller::configure(string optionName, string optionValue) {
//        std::cout<<"ofxARDrone::Controller::configure - " + optionName + ": " + optionValue);
//        
//        // TODO:
//        Command command;
//        command.setName("CONFIG");
//        queueCommand(command);
//    }
    
    
    //--------------------------------------------------------------
    void Controller::takeOff(bool b, int revertMillis ) {
        std::cout<<"ofxARDrone::Controller::takeOff -  + toString(b)";
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        drone->state.setTakingOff(b, revertMillis);
    }
    
    //--------------------------------------------------------------
    void Controller::land(bool b, int revertMillis ) {
        std::cout<<"ofxARDrone::Controller::land -  + toString(b)";
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        drone->state.setLanding(b, revertMillis);
    }
    
    
    //--------------------------------------------------------------
    void Controller::emergency(int on) {
        std::cout<<"ofxARDrone::Controller::emergency -  + toString(on)";
        
        Command command;
        command.setName("REF");
        
        msa::BitField32 bitField;
        bitField.clearAll();
        bitField.setBit(18);
        bitField.setBit(20);
        bitField.setBit(22);
        bitField.setBit(24);
        bitField.setBit(28);
        bitField.setBit(8, on);
        command.addInt(bitField);
        
        queueCommand(command);
    }
    
    //--------------------------------------------------------------
    void Controller::animation(animations anim, int speed){
        if(speed == 0){
            speed = AnimationDefaultDurations[anim];
        }
        
        Command command;
        command.setName("CONFIG");
        command.addString("control:flight_anim");
        //command.addString("toString(anim,0)+","+toString(speed,0));
        
        //queueCommand("AT*CONFIG="+toString(drone->sequenceNumber++)+",\"control:flight_anim\",\""+toString(anim,0)+","+toString(speed,0)+"\"");
    }
    
    
    //--------------------------------------------------------------
    void Controller::calibrateHorizontal(bool b, int revertMillis ) {
        std::cout<<"ofxARDrone::Controller::calibrateHorizontal -  + toString(b)";
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        drone->state.setCalibratingHorizontal(b, revertMillis);
    }
    
    //--------------------------------------------------------------
    void Controller::calibrateMagnetometer(bool b, int revertMillis ) {
        std::cout<<"ofxARDrone::Controller::calibrateMagnetometer -  + toString(b)";
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }
        drone->state.setCalibratingMagnetometer(b, revertMillis);
    }
    

    //--------------------------------------------------------------
    void Controller::exitBootstrap() {
        std::cout<<"ofxARDrone::Controller::exitBootstrap";
        
        Command configCommand;
        configCommand.setName("CONFIG");
        configCommand.addString("general:navdata_demo");
        configCommand.addString("TRUE");
        queueCommand(configCommand);

//        queueCommand("AT*CONFIG="+toString(drone->sequenceNumber++)+",\"general:navdata_demo\",\"TRUE\"");
        
        Command command;
        command.setName("CTRL");
        command.addInt(5);
        queueCommand(command);
    }
    
    
    //--------------------------------------------------------------
    void Controller::sendAck() {
        std::cout<<"ofxARDrone::Controller::sendAck";
        queueCommand("AT*CTRL=0");
    }

    
    //--------------------------------------------------------------
    void Controller::resetCommunicationWatchdog() {
//        std::cout<<"ofxARDrone::Controller::resetCommunicationWatchdog");
        
        queueCommand("AT*COMWDG");
    }

    
    //--------------------------------------------------------------
    void Controller::queueCommand(Command &command) {
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }

        string commandString = command.getString(drone->sequenceNumber);
        
        queueCommand(commandString);
        drone->sequenceNumber++;
    }
    
    
    //--------------------------------------------------------------
    void Controller::queueCommand(string cs) {
        std::cout<<"ofxARDrone::Controller::queueCommand - " + cs;
        
        cs += 0x0D;
        cs += 0x0A; // TODO: is this one needed?
        commandString += cs;
    }
    
    
    //--------------------------------------------------------------
    unsigned long Controller::getMillisSinceLastSend() {
        return 1 ;//ofGetElapsedTimeMillis() - millisOfLastSend;
    }

    

    
}
