/*
 Copyright (c) 2010, Hector Sanchez-Pajares
 Aer Studio http://www.aerstudio.com
 All rights reserved.
 
 
 This is a block for OSC Integration for Cinder framework developed by The Barbarian Group, 2010
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

/*
 Based on addon ofxOsc for openFrameworks
 by Damian Stewart
 http://addons.openframeworks.cc/projects/show/ofxosc
 http://frey.co.nz/old/projects/ofxosc/
 
 */

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/System.h"
#include "OscSender.h"
#include "cinder/osc/Osc.h"
using Receiver = cinder::osc_new::ReceiverUdp;
using protocol = asio::ip::udp;
using Sender = cinder::osc_new::SenderUdp;


const uint16_t localPort = 10002;

const std::string destinationHost = "127.0.0.1";
const uint16_t destinationPort = 1234;
using namespace ci;
using namespace ci::app;
using namespace std;

class OSCSkipPulseGeneratorApp : public App {
  public:
    OSCSkipPulseGeneratorApp();
	void setup();
	void update();
	void draw();
    static void prepareSettings( Settings *settings );
    void send(uint s);

	osc::Sender sender;

    uint    mFramesToSkip = 1;
    float   mLastFrame;
    int     mPulseValue;
    
    Receiver mReceiver;
    bool    mIsConnected;
};

OSCSkipPulseGeneratorApp::OSCSkipPulseGeneratorApp()
: mReceiver( localPort ), mIsConnected( false )
{
}

void OSCSkipPulseGeneratorApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate(30.f);
    gl::enableVerticalSync();
}


void OSCSkipPulseGeneratorApp::setup()
{
    sender.setup( destinationHost, destinationPort, true );
    
    mReceiver.setListener( "/skippulsegenerator/1",
                          [&]( const osc_new::Message &msg ){
                              if (msg.getNumArgs() < 1) {
                                  return;
                              }
                              cinder::osc_new::ArgType tp = msg[0].getType();
                              uint skip = -1;
                              switch (tp) {
                                  case cinder::osc_new::ArgType::FLOAT:
                                      skip = (uint) msg[0].flt();
                                      break;
                                  case cinder::osc_new::ArgType::INTEGER_32:
                                      skip = (uint) msg[0].int32();
                                      break;
                                  case cinder::osc_new::ArgType::INTEGER_64:
                                      skip = (uint) msg[0].int64();
                                      break;
                                  case cinder::osc_new::ArgType::DOUBLE:
                                      skip = (uint) msg[0].dbl();
                                      break;
                                  default:
                                      break;
                              }
                              if (skip == -1 || skip == mFramesToSkip) {
                                  return;
                              }
                              mFramesToSkip = skip;
                              mLastFrame = getElapsedFrames();
                          });
    try {
        // Bind the receiver to the endpoint. This function may throw.
        mReceiver.bind();
    }
    catch( const cinder::osc_new::Exception &ex ) {
        //CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
        quit();
    }
    
    // UDP opens the socket and "listens" accepting any message from any endpoint. The listen
    // function takes an error handler for the underlying socket. Any errors that would
    // call this function are because of problems with the socket or with the remote message.
    mReceiver.listen(
                     []( asio::error_code error, protocol::endpoint endpoint ) -> bool {
                         if( error ) {
                             //CI_LOG_E( "Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint );
                             return false;
                         }
                         else
                             return true;
                     });
    mIsConnected = true;
}

void OSCSkipPulseGeneratorApp::send(uint s) {
    osc::Message message;
    message.setAddress("/isadora/1");
    message.addIntArg(s);
    sender.sendMessage(message);
}

void OSCSkipPulseGeneratorApp::update()
{
    
    auto elFrames = getElapsedFrames() - mLastFrame;
    if (elFrames >= mFramesToSkip) {
        mLastFrame = getElapsedFrames();
        mPulseValue = 2;
    } else {
        mPulseValue = 1;
    }
    
    send (mPulseValue);
    
    getWindow()->setTitle("Pulse every " + to_string((int) getFrameRate()) + " fps: 1, skip " + to_string(mFramesToSkip) +" then 2");
}

void OSCSkipPulseGeneratorApp::draw()
{
	gl::clear();
    if (1 == mPulseValue)
        gl::color( Color::gray( 0.5f ) );
    else if (2 == mPulseValue)
        gl::color( Color::white());
    
	gl::drawSolidRect(Rectf(vec2(0), vec2(getWindowWidth(), getWindowHeight())));
}

CINDER_APP( OSCSkipPulseGeneratorApp, RendererGl, OSCSkipPulseGeneratorApp::prepareSettings  )
