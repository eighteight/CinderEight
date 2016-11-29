#ifndef _FBO_SHADER_APP
#define _FBO_SHADER_APP

// File includes
#include "cinder/Cinder.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/app/AppBasic.h"
#include "Resources.h"

// Imports
using namespace ci;
using namespace ci::app;
using namespace std;

class fboShaderApp : public AppBasic
{

public:

	// Cinder callbacks
	void draw();
	void keyDown(KeyEvent event);
	void prepareSettings(Settings *settings);
	void setup();
	void update();

private:
	
	// Constants
	static const int FBO_WIDTH = 1024;
	static const int FBO_HEIGHT = 768;
	static const int ITERATIONS = 4;	// Should always be even

	// Frame buffer objects
	int fboPing;
	int fboPong;
	gl::Fbo FBOs[2];

	// Shaders
	gl::GlslProg shaderRender;
	gl::GlslProg shaderProcess;

	// Background texture
	gl::Texture texImage;

	// Randomizer
	Rand random;

	// Save frames flag (record output)
	bool bSaveFrames;

	// True renders input to screen
	bool bShowInput;

};

#endif
