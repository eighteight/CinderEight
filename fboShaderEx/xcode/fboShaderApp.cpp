// Include the header
#include "fboShaderApp.h"

// Draw
void fboShaderApp::draw()
{

	// Clear screen and set up viewport
	gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
	gl::setMatricesWindow(getWindowSize());
	gl::setViewport(getWindowBounds());

	// We're in input-only mode
	if (bShowInput){
		// Render the raw input
		gl::draw(FBOs[fboPing].getTexture());

	}
	else {

		// Bind the FBO we last rendered as a texture
		FBOs[fboPing].bindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Start and configure the render shader
		shaderRender.bind();

		// This is the image we've been drawing
		shaderRender.uniform("texture", 0);

		// This is the texture we'll be refracting
		shaderRender.uniform("srcTexture", 1);

		// Pass 
		shaderRender.uniform("width", (float)FBO_WIDTH);
		shaderRender.uniform("height", (float)FBO_HEIGHT);

		// Draw rectangle
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		gl::drawSolidRect(FBOs[fboPing].getBounds());

		// Stop shader
		shaderRender.unbind();

	}

	// Save each frame to PNG if we're in record mode
	//if (bSaveFrames) writeImage(::getAppPath() + "frames/frame_" + to_string((long double)getElapsedFrames()) + ".png", copyWindowSurface());

}

// Handles key press
void fboShaderApp::keyDown(KeyEvent event) {
	// Key on key
	switch (event.getChar()) {
		case KeyEvent::KEY_ESCAPE:
			// ESC quits
			quit();
		break;
		case KeyEvent::KEY_i:
			// I toggles input view
			bShowInput = !bShowInput;
			break;
		case KeyEvent::KEY_r:
			// R toggles recording
			bSaveFrames = !bSaveFrames;
		break;
	}
}

void fboShaderApp::mouseDrag (MouseEvent event){
    mousePosition = event.getPos();
}

void fboShaderApp::mouseDown (MouseEvent event){
    mousePosition = event.getPos();
}

void fboShaderApp::fileDrop( FileDropEvent event ){	
	// use the last of the dropped files
	//mFile = event.getFile( event.getNumFiles() - 1 );
    
	try { 
		// try loading image file
		texImage = gl::Texture( loadImage( mFile ) );
	}
	catch(...) {
		// otherwise, try loading QuickTime video
		play( mFile );
	}	
}

void fboShaderApp::play( const string &file ) 
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

void fboShaderApp::playNext()
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

// Sets up screen
void fboShaderApp::prepareSettings(Settings *settings)
{

	// Set window shape and frame rate
	settings->setWindowSize(FBO_WIDTH, FBO_HEIGHT);
	settings->setFrameRate(60.0f);
	settings->setResizable(false);

}

// Set up
void fboShaderApp::setup()
{

	// Set flags
	bSaveFrames = false;
	bShowInput = false;

	// Smoothes edges on the random circles
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Load shaders
	shaderRender = gl::GlslProg(loadResource(RES_PASS_THRU_VERT), loadResource(RES_RENDER_FRAG));
	shaderProcess = gl::GlslProg(loadResource(RES_PASS_THRU_VERT), loadResource(RES_PROCESS_FRAG));

	// Load image texture and bind at 1
    
    try {
		std::string path = "/Users/gusev/Dropbox/little-tragedies/city1.png";//getOpenFilePath( "", ImageIo::getLoadExtensions() );
		if( ! path.empty() ) {
			texImage = gl::Texture( loadImage( path ) );
		}
	}
	catch( ... ) {
		console() << "unable to load the texture file!" << std::endl;
	}
    
	texImage = gl::Texture(loadImage(loadResource(RES_TEXTURE)));
	texImage.setWrap(GL_REPEAT, GL_REPEAT);
	texImage.setMinFilter(GL_LINEAR);
	texImage.setMagFilter(GL_LINEAR);
	texImage.bind(1);
	
	// Create FBO format which will smooth out the image
	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.setSamples(4);
	format.setCoverageSamples(8);
	
	// Set up frame buffer objects
	fboPing = 0;
	fboPong = 1;
	FBOs[fboPing] = gl::Fbo(FBO_WIDTH, FBO_HEIGHT, format);
	FBOs[fboPong] = gl::Fbo(FBO_WIDTH, FBO_HEIGHT, format);
    mousePosition = Vec2i(0,0);
}

// Update
void fboShaderApp::update()
{	
    
    // update movie texture if necessary
	if(mMovie) { 
		// get movie surface
		Surface surf = mMovie.getSurface();
        
		// copy surface into texture
		if(surf){
			texImage = gl::Texture( surf );
            texImage.bind(1);
        }
        
		// play next movie in directory when done
		if( mMovie.isDone() ) 
			playNext();
	}

	// Set up viewport
	gl::setMatricesWindow(FBOs[fboPing].getSize(), false);
	gl::setViewport(FBOs[fboPing].getBounds());

	// Loop through iteration count
	for (int i = 0; i < ITERATIONS; i++)
	{

		// Swap FBO indexes
		fboPing = (fboPing + 1) % 2;
		fboPong = (fboPong + 1) % 2;

		// Bind the "ping" FBO so we can draw onto it
		FBOs[fboPing].bindFramebuffer();

		// Bind the "pong" FBO as a texture to 
		// send to the shader
		FBOs[fboPong].bindTexture();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


		/****** Begin process shader configuration ******/

		// Bind the process shader
		shaderProcess.bind();

		// This was the "ping" FBO we drew onto 
		// in the last iteration
		shaderProcess.uniform("texture", 0); 

		// This controls the definition of edges in 
		// the shader (higher is softer)
		shaderProcess.uniform("dampen", 350.0f);

		// These are used in a mutant variant of the 
		// Gray-Scott reaction-diffusion equation
		shaderProcess.uniform("ru", 0.33f); 
		shaderProcess.uniform("rv", 0.1f);
		shaderProcess.uniform("k", 0.06f);
		shaderProcess.uniform("f", 0.25f);

		// We pass in the width and height to convert normalized 
		// coordinates to screen coordinates
		shaderProcess.uniform("width", (float)FBO_WIDTH);
		shaderProcess.uniform("height", (float)FBO_HEIGHT);

		/****** End process shader configuration ******/


		// Draw the shader output onto the "ping" FBO
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		gl::drawSolidRect(FBOs[fboPing].getBounds());

		// Stop the shader
		shaderProcess.unbind();

		// Draw a red circle randomly on the screen
		random.randomize();
		RectMapping windowToFBO(getWindowBounds(), FBOs[fboPing].getBounds());
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		//gl::drawSolidCircle(windowToFBO.map(Vec2f(random.nextFloat(0.0f, (float)FBO_WIDTH), random.nextFloat(0.0f, (float)FBO_HEIGHT))), random.nextFloat(1.0f, 30.0f), 64);
		gl::drawSolidCircle(windowToFBO.map(Vec2f(mousePosition.x, mousePosition.y)), random.nextFloat(1.0f, 30.0f), 64);
	
		// TO DO: Draw anything you want here in red. 
		//		  It will refract the image texture.

		// Unbind the FBO to stop drawing on it
		FBOs[fboPing].unbindFramebuffer();

	}

}

// Runs the application
CINDER_APP_BASIC(fboShaderApp, RendererGl)
