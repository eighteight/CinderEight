/*
 Copyright (C)2010 Paul Houx
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "cinder/app/AppBasic.h"
#include "ThreadedImage.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ThreadedImageLoaderApp : public AppBasic {
  public:
	ThreadedImage	mImage;
    
    void setup();
	void shutdown();
    void update();
    void draw();  

	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );
};

void ThreadedImageLoaderApp::setup()
{
	// set desired framerate
    setFrameRate(60.0);

	// load first image
	mImage.load("../data/sunset.jpg");
}

void ThreadedImageLoaderApp::shutdown()
{
}

void ThreadedImageLoaderApp::update()
{
	mImage.update();
}

void ThreadedImageLoaderApp::draw()
{
    gl::clear();

	// draw image
	gl::pushModelView();
		gl::translate( getWindowCenter() );
		gl::rotate( Vec3f(0.0f, 0.0f, 10.0f * (float) getElapsedSeconds()) );
		gl::translate( -0.5f * Vec2f(mImage.getSize()) );

		mImage.draw();
	gl::popModelView();
}

void ThreadedImageLoaderApp::keyDown(KeyEvent event)
{
	switch(event.getCode())
	{
	case KeyEvent::KEY_1:
		mImage.load("../data/sunset.jpg");
		break;
	case KeyEvent::KEY_2:
		mImage.load("http://www.cowlumbus.nl/files/empire_state_building.jpg");
		break;
	case KeyEvent::KEY_3:
		mImage.load("http://www.cowlumbus.nl/files/aletschgletscher.jpg");
		break;
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	}
}

void ThreadedImageLoaderApp::fileDrop(FileDropEvent event)
{
	// only load last file dropped
	size_t n = event.getNumFiles();
	mImage.load( event.getFile(n-1) );
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( ThreadedImageLoaderApp, RendererGl )