/*
 Copyright (c) 2014, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
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

//https://medium.com/community-play-3d/god-rays-whats-that-5a67f26aeac2#.6yagjfspl

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/gl/Light.h"
#include "Resources.h"
#include "cinder/Perlin.h"
#include "cinder/params/Params.h"
#include "cinder/Capture.h"

//#define INPUT_DEVICE "Scarlett 2i2 USB"
#define INPUT_DEVICE "Soundflower (2ch)"


using namespace ci;
using namespace ci::app;
using namespace std;

class GodRaysApp : public AppNative {
public:
    void prepareSettings( Settings* settings );
    
    void setup();
    void shutdown();
    void update();
    void draw();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseUp( MouseEvent event );
    void keyDown( KeyEvent event );
    void mouseWheel( MouseEvent event );
    void resize();
    
private:

    cinder::gl::GlslProgRef		    mShader;

    gl::TextureRef		mTexture;
    gl::Texture::Format	mTextureFormat;
    
    Perlin				mPerlin;
    uint32              mPerlinMove;

    float mFrameRate;
    ci::params::InterfaceGl		mParams;
    CaptureRef			mCapture;

};

void GodRaysApp::prepareSettings(Settings* settings)
{
    settings->setFullScreen(false);
    settings->setWindowSize(1280, 720);
}

void GodRaysApp::mouseWheel( MouseEvent event )
{
}

void GodRaysApp::setup()
{
    
    mPerlin = Perlin( 4, 0 );
    mPerlinMove = 0;
    mFrameRate	= 0.0f;
    mParams = params::InterfaceGl( "Params", Vec2i( 200, 100 ) );
	mParams.addParam( "Frame rate",	&mFrameRate,"", true);
    
    // create texture format (wrap the y-axis, clamp the x-axis)
    mTextureFormat.setWrapS( GL_CLAMP );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );

    try {
        mShader = gl::GlslProg::create( loadResource( GLSL_VERT1 ), loadResource( GLSL_FRAG1 ) );
    }
    catch( const std::exception& e ) {
        console() << e.what() << std::endl;
        quit();
        return;
    }

    setFrameRate(30.0f);

    //fog
    GLfloat density = 0.1;
    GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
    glEnable (GL_DEPTH_TEST); //enable the depth testing
    glEnable (GL_FOG);
    glFogi (GL_FOG_MODE, GL_EXP2);
    glFogfv (GL_FOG_COLOR, fogColor);
    glFogf (GL_FOG_DENSITY, density);
    glHint (GL_FOG_HINT, GL_NICEST);
    
    for( auto device = Capture::getDevices().begin(); device != Capture::getDevices().end(); ++device ) {
        console() << "Device: " << (*device)->getName() << " "
#if defined( CINDER_COCOA_TOUCH )
        << ( (*device)->isFrontFacing() ? "Front" : "Rear" ) << "-facing"
#endif
        << std::endl;
    }
    
    try {
        mCapture = Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ... ) {
        console() << "Failed to initialize capture" << std::endl;
    }

}

void GodRaysApp::shutdown()
{

}

void GodRaysApp::update()
{
    mFrameRate = getAverageFps();
    if( mCapture && mCapture->checkNewFrame() ) {
        mTexture = gl::Texture::create( mCapture->getSurface() );
    }
}


void GodRaysApp::draw()
{
    
    gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
    if( mTexture ) {
        glPushMatrix();
        gl::draw( mTexture );
        glPopMatrix();
    }

    return;
    gl::pushMatrices();
    {

        // bind shader
        mShader->bind();
        mShader->uniform("uLeftTex", 0);

        mTexture->bind(0);
        
        // draw mesh using additive blending
        gl::enableAdditiveBlending();

        gl::color( Color(1, 1, 1) );


        gl::disableAlphaBlending();

        // unbind textures and shader
        mTexture->unbind();
        mShader->unbind();
    }

    gl::popMatrices();
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    mParams.draw();
    
}

void GodRaysApp::mouseDown( MouseEvent event )
{

}

void GodRaysApp::mouseDrag( MouseEvent event )
{
}

void GodRaysApp::mouseUp( MouseEvent event )
{
}

void GodRaysApp::keyDown( KeyEvent event )
{
    // handle key down
    switch( event.getCode() )
    {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_F4:
            if( event.isAltDown() )
                quit();
            break;
        case KeyEvent::KEY_LEFT:
            
            break;
        case KeyEvent::KEY_RIGHT:
            break;
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_o:
            break;
        case KeyEvent::KEY_p:
            break;
        case KeyEvent::KEY_s:
            
            break;
    }
}

void GodRaysApp::resize()
{

}

CINDER_APP_NATIVE( GodRaysApp, RendererGl )