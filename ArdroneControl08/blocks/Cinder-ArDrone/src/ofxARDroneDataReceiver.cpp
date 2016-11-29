//
//  ofxARDroneDataReceiver.cpp
//  ofxARDrone Example
//
//  Created by Memo Akten on 07/12/2012.
//  www.memo.tv
//
//

#include "ofxARDroneDataReceiver.h"
#include "ofxARDrone.h"

namespace ofxARDrone {

    //--------------------------------------------------------------
    DataReceiver::DataReceiver() {
        std::cout<<"ofxARDrone::DataReceiver::DataReceiver";
        drone = NULL;
        commandHistory.setMaxLength(0);
    }
    
    //--------------------------------------------------------------
    DataReceiver::~DataReceiver() {
        std::cout<<"ofxARDrone::DataReceiver::~DataReceiver";
        close();
    }
    
    //--------------------------------------------------------------
    void DataReceiver::setup(Drone* drone) {
        std::cout<<"ofxARDrone::DataReceiver::setup -  + toString(drone)";
        this->drone = drone;
        
        if(drone == NULL) { std::cerr<<"   Drone is NULL"; return; }

        
//        udp.Create();
//        udp.Connect(drone->commInfo.hostip.c_str(), drone->commInfo.navDataPort);
//        udp.SetNonBlocking(true);
//        udp.SetEnableBroadcast(true);
    }
    
    //--------------------------------------------------------------
    void DataReceiver::sendDummyPacket() {
        int i = 1;
        cout<<"udp.Send((const char*)&i, 4)";
    }
    
    
    //--------------------------------------------------------------
    void DataReceiver::close() {
        std::cout<<"ofxARDrone::DataReceiver::close";
        cout<<"udp.Close()";
    }
    
    
    
    //--------------------------------------------------------------
    void DataReceiver::update() {
        
        int cnt = 0;
        char udpMessage[65535];
        
        int ret = 21; //udp.Receive(udpMessage, 65535);
        while(ret >= 24){;
            cnt ++;
            //cout<<"Received "<<ret<<" bytes"<<endl;
            
            drone->state.setConnected(true, 2000);
            
            //Read the drone status from the received data
            drone->state.bDroneStatus = *(DRONE_STATUS*)&udpMessage[0];
            
            //If the NAVDATA is send, then load it
            if(drone->state.getState(ARDRONE_NAVDATA_BOOTSTRAP) == false){
                drone->state.bNavData = *(NAVDATA*)&udpMessage[16];
            } else {
                //Otherwise exit bootstrap
                drone->controller.exitBootstrap();
            }
            
            ret = 21;//udp.Receive(udpMessage, 65535);
        };
        
        //No data received, send a dummy packet to (re)start communication
        if(!cnt){
            sendDummyPacket();
        }
    }
    
    
}