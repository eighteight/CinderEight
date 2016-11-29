/*
 Copyright (C)2011 Paul Houx
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, 
 are permitted without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma comment(lib, "QTMLClient.lib")
#pragma comment(lib, "CVClient.lib")

#include "cinder/Filesystem.h"
#include "cinder/ImageIo.h"
#include "cinder/Matrix.h"
#include "cinder/Surface.h"
#include "cinder/Utilities.h"

#include "cinder/app/AppBasic.h"

#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/qtime/QuickTime.h"

#include "Resources.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class PostProcessingApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );

	void play( const string &file );
	void playNext();
    void	resetFBOs(int);
    void drawFBO();
    
protected:
	gl::Texture				mImage;
	gl::GlslProg			mShader;
	qtime::MovieSurface		mMovie;
	string					mFile;
    gl::Fbo			        mFBO;
    int                     side;
    
    gl::Fbo::Format         format;
    float                   glitchTime, startGlitchTime;
};

void PostProcessingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(1024, 768);
	settings->setFrameRate(30.0f);
	settings->setTitle("Post-processing Video Player");
}

void PostProcessingApp::setup()
{
    
    try {
		std::string path = getOpenFilePath( "", ImageIo::getLoadExtensions() ).string();
		if( ! path.empty() ) {
			mImage = gl::Texture( loadImage( path ) );
		}
	}
	catch( ... ) {
		console() << "unable to load the texture file!" << std::endl;
	}

	// load post-processing shader
	//  adapted from a shader by Iñigo Quílez ( http://www.iquilezles.org/ )
	try {
		mShader = gl::GlslProg( loadResource( RES_POSTPROCESS_VERT ), loadResource( RES_POSTPROCESS_FRAG ) );
	}
	catch( gl::GlslProgCompileExc e ) {
		console() << "Could not compile shader: " << e.what() << std::endl;
	}
	catch(...) {}
    
    glitchTime = startGlitchTime = 0.0f;

	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, 2);
	format.setMinFilter( GL_NEAREST );
	format.setMagFilter( GL_NEAREST );
	format.setColorInternalFormat( GL_RGBA32F_ARB );
    format.enableMipmapping( true );
    format.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
    side = 1;

    resetFBOs(side);
}

void PostProcessingApp::resetFBOs(int side){
	mFBO = gl::Fbo( side, side, format );
}

void PostProcessingApp::update()
{
	// update movie texture if necessary
	if(mMovie) { 
		// get movie surface
		Surface surf = mMovie.getSurface();

		// copy surface into texture
		if(surf)
			mImage = gl::Texture( surf );

		// play next movie in directory when done
		if( mMovie.isDone() ) 
			playNext();
	}

    glitchTime = (float)getElapsedSeconds()-startGlitchTime;

    if (mImage){
        if (side <= 1024 && ((int)getElapsedFrames()%5 ==0)){
            side +=2;
            resetFBOs(side);
        }

        drawFBO();
    }
}

void PostProcessingApp::drawFBO(){
    mFBO.bindFramebuffer();
    Area viewport = gl::getViewport();
    gl::setViewport(mFBO.getBounds());
    mImage.setFlipped(true);
    gl::color( Color::white() );
    gl::draw( mImage, getWindowBounds() );
    gl::setViewport(viewport);
    mFBO.unbindFramebuffer();

}

void PostProcessingApp::draw()
{
	// clear window
	gl::clear();
	mFBO.bindTexture(0,0);
	// bind shader and set shader variables
	mShader.bind();
	mShader.uniform( "tex0", 0 );
	mShader.uniform( "time", glitchTime );
    mShader.uniform( "fillColor", Vec4f(0.1,0.2,0.2,0.5) );

	// draw image or video
	gl::color( Color::white() );
	gl::draw( mFBO.getTexture(), getWindowBounds() );

	// unbind shader
	mShader.unbind();
    mFBO.unbindTexture();
}

void PostProcessingApp::keyDown( KeyEvent event )
{	
	switch( event.getCode() ) {
		case KeyEvent::KEY_ESCAPE:
			quit();
			break;
		case KeyEvent::KEY_F4:
			if(event.isAltDown()) {
				quit();
			} 
			break;
		case KeyEvent::KEY_RETURN:
			if(event.isAltDown()) {
				setFullScreen( !isFullScreen() );
			}
			break;
        case KeyEvent::KEY_0:
            startGlitchTime = getElapsedSeconds();
            side = 1;
			break;
	}
}

void PostProcessingApp::fileDrop( FileDropEvent event )
{	
	// use the last of the dropped files
	mFile = event.getFile( event.getNumFiles() - 1 ).string();

	try { 
		// try loading image file
		mImage = gl::Texture( loadImage( mFile ) );
	}
	catch(...) {
		// otherwise, try loading QuickTime video
		play( mFile );
	}	
}

void PostProcessingApp::play( const string &file ) 
{
	try {
		// try loading QuickTime movie
		mMovie = qtime::MovieSurface( file );
		mMovie.play();
	}
	catch(...) {}

	// keep track of file
	mFile = file;
}

void PostProcessingApp::playNext()
{
	// get directory
	fs::path path( getPathDirectory( mFile ) );

	// list *.mov files
	vector<string> files;
	string filter( ".mov" );

	fs::directory_iterator end_itr;
	for(fs::directory_iterator itr(path);itr!=end_itr;++itr) {
		// skip if not a file
		if( !boost::filesystem::is_regular_file( itr->status() ) ) continue;
		
		// skip if no match
		if( itr->path().filename().string().find( filter ) == string::npos ) continue;

		// file matches, store it
		files.push_back( itr->path().string() );
	}

	// check if playable files are found
	if( files.empty() ) return;

	// play next file
	vector<string>::iterator itr = find(files.begin(), files.end(), mFile);
	if( itr == files.end() ) {
		play( files[0] );
	}
	else {
		++itr;
		if( itr == files.end() ) 
			play( files[0] );
		else play( *itr );
	}
}

CINDER_APP_BASIC( PostProcessingApp, RendererGl )
