#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/Rand.h"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "cinder/Perlin.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Capture.h"

#include "cinderSyphon.h"

#include "Resources.h"

#include "CinderFreenect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 512
#define HEIGHT 512
#define PARTICLES 256

#define SYWIDTH 800
#define SYHEIGHT 600

class VideoToParticles : public AppNative
{

public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void initFbo();
	void prepareSettings(Settings *settings);
	void keyDown(KeyEvent event);
	void drawText();
	void loadShaders();
    void initCapture();
    void drawVideo();
	
private:
	int m_pos;
	int m_bufferIn;
	int m_bufferOut;
	int m_frameCtr;
	
	bool m_drawTextures;
	bool m_isFullScreen;
	bool m_createParticles;
	
	Perlin m_perlin;
	
	Vec3f m_vertPos;
	
	gl::VboMesh m_vbo;
	gl::Fbo m_fbo[2];
	gl::Fbo m_fboSy;
	
	gl::GlslProg m_shdrPos;
	gl::GlslProg m_shdrVel;
	gl::GlslProg m_shdrDbg;
	
	gl::Texture m_texPos;
	gl::Texture m_texVel;
	gl::Texture m_texInfo;
	gl::Texture m_texNoise;
	gl::Texture m_texSprite;
	gl::TextureRef m_texSyRef;
	
	syphonClient m_clientSyphon; //our syphon client
	syphonServer m_srvSyphon;
	
	float m_parts_speed;
	float m_parts_direction;
	float m_parts_size;
	float m_parts_numparts;
	float m_parts_rotspeed;
	
	//KinectRef m_kinect;
	gl::Texture m_depthTex;

    CaptureRef			mCapture;
    gl::TextureRef		mTexture;
};

void VideoToParticles::initFbo()
{
	m_pos = 0;
	m_bufferIn = 0;
	m_bufferOut = 1;
	
	m_fbo[0].bindFramebuffer();
	m_fbo[1].bindFramebuffer();
	
	// POS
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	
	gl::setMatricesWindow(m_fbo[0].getSize());
	gl::setViewport(m_fbo[0].getBounds());
	
	glClearColor(.0f, .0f, .0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_texPos.enableAndBind();
	gl::draw(m_texPos, m_fbo[0].getBounds());
	m_texPos.unbind();
	
	// VEL
	glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_texVel.enableAndBind();
	gl::draw(m_texVel, m_fbo[0].getBounds());
	m_texVel.unbind();
	
	// INFO
	glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_texInfo.enableAndBind();
	gl::draw(m_texInfo, m_fbo[0].getBounds());
	m_texInfo.unbind();
	
	m_fbo[0].unbindFramebuffer();
	m_fbo[1].unbindFramebuffer();
	
	m_texInfo.disable();
	m_texVel.disable();
	m_texPos.disable();
}

void VideoToParticles::loadShaders()
{
	try {
		m_shdrPos = gl::GlslProg(loadFile("../../../resources/shdrPosV.glsl"), loadFile("../../../resources/shdrPosF.glsl"));
		m_shdrVel = gl::GlslProg(loadFile("../../../resources/shdrVelV.glsl"), loadFile("../../../resources/shdrVelF.glsl"));
		m_shdrDbg = gl::GlslProg(ci::app::loadResource(DBG_VS), ci::app::loadResource(DBG_FS));
	
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
}

void VideoToParticles::initCapture(){
    // print the devices
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

void VideoToParticles::setup()
{
	gl::clear();
	
	loadShaders();
	
	m_drawTextures = false;
	m_isFullScreen = false;
	m_frameCtr = 0;
	
	m_perlin = Perlin(32, clock() * .1f);
	
	Surface32f posSurf(PARTICLES, PARTICLES, true);
	Surface32f velSurf(PARTICLES, PARTICLES, true);
	Surface32f infoSurf(PARTICLES, PARTICLES, true);
	Surface32f noiseSurf(PARTICLES, PARTICLES, true);
	
	Surface32f::Iter iter = posSurf.getIter();
	
	float winW = getWindowWidth();
	float winH = getWindowHeight();
	
	// generate some initial values for all of the particles
	while (iter.line())
	{
		while (iter.pixel())
		{
		
			m_vertPos = Vec3f(Rand::randFloat(winW) / winW, Rand::randFloat(winH) / winH, .0f);
			
			
			float nX = iter.x() * .005f;
			float nY = iter.y() * .005f;
			float nZ = app::getElapsedSeconds() * 0.1f;
			Vec3f v(nX, nY, nZ);
			float noise = m_perlin.fBm(v);
			float angle = noise * 15.0f;
			Vec2f vel(Rand::randFloat(-.0005f, .0005f), Rand::randFloat(-.0005f, .0005f));
			
			noiseSurf.setPixel(iter.getPos(),
				ColorA(cos(angle) * Rand::randFloat(.00005f,.0002f),
					  sin(angle) * Rand::randFloat(.00005f,.0002f),
					  0.0f, 1.0f ));
			
			posSurf.setPixel(iter.getPos(),
				ColorA(m_vertPos.x, m_vertPos.y, m_vertPos.z, Rand::randFloat(.00005f, .0002f)));
							 
			velSurf.setPixel(iter.getPos(), Color(vel.x, vel.y, Rand::randFloat(.01f, 1.00f)));
			infoSurf.setPixel(iter.getPos(), ColorA(Rand::randFloat(.007f, 1.0f), 1.0f, .0f, 1.00f));
		}
	}
	
	gl::Texture::Format tFormat;
	
//	tFormat.setInternalFormat(GL_RGBA16F_ARB);
	gl::Texture::Format tFormatSmall;
//	tFormat.setInternalFormat(GL_RGBA8);
	
	m_texSprite = gl::Texture(loadImage(loadResource("cross2.png")), tFormatSmall);
	
	GLenum interp = GL_NEAREST;
	
	m_texNoise = gl::Texture(noiseSurf, tFormatSmall);
	m_texNoise.setWrap(GL_REPEAT, GL_REPEAT);
	m_texNoise.setMinFilter(interp);
	m_texNoise.setMagFilter(interp);
	
	m_texVel = gl::Texture(velSurf, tFormat);
	m_texVel.setWrap(GL_REPEAT, GL_REPEAT);
	m_texVel.setMinFilter(GL_NEAREST);
	m_texVel.setMagFilter(GL_NEAREST);
	
	m_texPos = gl::Texture(posSurf, tFormat);
	m_texPos.setWrap(GL_REPEAT, GL_REPEAT);
	m_texPos.setMinFilter(GL_NEAREST);
	m_texPos.setMagFilter(GL_NEAREST);
	
	m_texInfo = gl::Texture(infoSurf, tFormatSmall);
	m_texInfo.setWrap(GL_REPEAT, GL_REPEAT);
	m_texInfo.setMinFilter(GL_NEAREST);
	m_texInfo.setMagFilter(GL_NEAREST);
	
	gl::Fbo::Format format;
	format.enableDepthBuffer(false);
	format.enableColorBuffer(true, 3);
	format.setMinFilter(GL_NEAREST);
	format.setMagFilter(GL_NEAREST);
	format.setWrap(GL_CLAMP, GL_CLAMP);
	format.setColorInternalFormat(GL_RGBA16F_ARB);
	
	gl::Fbo::Format format2;
	format2.enableDepthBuffer(false);
	format2.enableColorBuffer(true, 1);
	format2.setMinFilter(GL_LINEAR);
	format2.setMagFilter(GL_LINEAR);
	format2.setWrap(GL_CLAMP, GL_CLAMP);
	format2.setColorInternalFormat(GL_RGBA16F_ARB);
	
	// these two will be alternatingly be used as input and output of the calculations
	m_fbo[0] = gl::Fbo(PARTICLES, PARTICLES, format);
	m_fbo[1] = gl::Fbo(PARTICLES, PARTICLES, format);
	
	m_fboSy = gl::Fbo(SYWIDTH, SYHEIGHT, format2);
	
	//initFbo();
	
	vector<Vec2f> texCoords;
	vector<Vec3f> vertCoords, normCoords;
	vector<uint32_t> indices;
	
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();
	
	m_vbo = gl::VboMesh(PARTICLES*PARTICLES, PARTICLES*PARTICLES, layout, GL_POINTS);
	
	for (int x = 0; x < PARTICLES; x++)
	{
		for (int y = 0; y < PARTICLES; y++)
		{
			indices.push_back(x * PARTICLES + y);
			texCoords.push_back(Vec2f((float)x / (float)PARTICLES, (float)y / (float)PARTICLES));
		}
	}
	
	m_vbo.bufferIndices(indices);
	m_vbo.bufferTexCoords2d(0, texCoords);
	
	m_clientSyphon.setup();
    
	// in order for this to work, you must run simple server which is a syphon test application
    // feel free to change the app and server name for your specific case
    m_clientSyphon.set("qtz1", "Quartz Composer");
    
    m_clientSyphon.bind();
	
	m_srvSyphon.setName("parts");

	
	m_parts_speed = .0f;
	
	m_texSyRef = gl::Texture::create(SYWIDTH, SYHEIGHT);
	
	//console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
	
    initCapture();
	//m_kinect = Kinect::create();
	
	m_parts_speed = 0.1;
	m_parts_size = 1.0;
	m_parts_direction = 1.0;
	m_parts_numparts = 1.0;
	m_parts_rotspeed = 0.0;
	
}

void VideoToParticles::mouseDown( MouseEvent event )
{
}

void VideoToParticles::keyDown( KeyEvent event)
{
    
	if (event.getChar() == 't')
	{
		m_drawTextures = !m_drawTextures;
	}
	else if (event.getChar() == 'f')
	{
		setFullScreen(!isFullScreen());
	}
	else if (event.getChar() == event.KEY_SPACE)
	{
		m_createParticles = true;
	}
}

void VideoToParticles::update()
{
    if( mCapture && mCapture->checkNewFrame() ) {
		mTexture = gl::Texture::create( mCapture->getSurface() );
	}
    return;
	// we don't need to update the kinect every frame, it doesn't make much difference in appearance
//	if (getElapsedFrames() % 2 == 0 && m_kinect->checkNewDepthFrame())
//		m_depthTex = m_kinect->getDepthImage();
	
	///////
	
	m_fbo[m_bufferIn].bindFramebuffer();
	
	gl::setMatricesWindow(m_fbo[0].getSize());
	gl::setViewport(m_fbo[0].getBounds());
	GLenum buffer[3] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
	glDrawBuffers(3, buffer);
	
	// we use the 3 attachments of the FBO as input textures
	m_fbo[m_bufferOut].bindTexture(0, 0);
	m_fbo[m_bufferOut].bindTexture(1, 1);
	m_fbo[m_bufferOut].bindTexture(2, 2);
	m_texVel.bind(3);
	m_texPos.bind(4);
	
	// if we have a depth image, send it in as the second noise texture
    if (mTexture){
        mTexture->bind(6);
    } else {
        m_texNoise.bind(6);
    }
    
//	if (m_depthTex)
//		m_depthTex.bind(6);
//	else
//		m_texNoise.bind(6);
	
	m_texNoise.bind(5);
	
	m_shdrVel.bind();
	m_shdrVel.uniform("positions", 0);
	m_shdrVel.uniform("velocities", 1);
	m_shdrVel.uniform("information", 2);
	m_shdrVel.uniform("oVelocities", 3);
	m_shdrVel.uniform("oPositions", 4);
	m_shdrVel.uniform("texNoise", 5);
	m_shdrVel.uniform("texNoise2", 6);
	m_shdrVel.uniform("speed", m_parts_speed);
	m_shdrVel.uniform("direction", m_parts_direction);
	m_shdrVel.uniform("time", (float)getElapsedSeconds());
	
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0.0f, 0.0f); glVertex2f( 0.0f, 0.0f);
	glTexCoord2f( 0.0f, 1.0f); glVertex2f( 0.0f, PARTICLES);
	glTexCoord2f( 1.0f, 1.0f); glVertex2f( PARTICLES, PARTICLES);
	glTexCoord2f( 1.0f, 0.0f); glVertex2f( PARTICLES, 0.0f);
	glEnd();
	
	m_shdrVel.unbind();
	m_fbo[m_bufferOut].unbindTexture();
	m_texVel.unbind();
	m_texPos.unbind();
	m_texNoise.unbind();
	m_fbo[m_bufferIn].unbindFramebuffer();
    
    if (mTexture)
		mTexture->unbind();
    
//	if (m_depthTex)
//		m_depthTex.unbind();
	
	m_bufferIn = (m_bufferIn + 1) % 2;
	m_bufferOut = (m_bufferIn + 1) % 2;
	
}

void VideoToParticles::drawVideo()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	
	if( mTexture ) {
		glPushMatrix();
#if defined( CINDER_COCOA_TOUCH )
		//change iphone to landscape orientation
		gl::rotate( 90.0f );
		gl::translate( 0.0f, -getWindowWidth() );
        
		Rectf flippedBounds( 0.0f, 0.0f, getWindowHeight(), getWindowWidth() );
		gl::draw( mTexture, flippedBounds );
#else
		gl::draw( mTexture );
#endif
		glPopMatrix();
	}
}

void VideoToParticles::draw()
{
    
    drawVideo();
    return;
	// we need to call this to draw billboards instead of points
	glEnable(GL_POINT_SPRITE);
		
	// just a debug view to see the kinect depth image. press 't' to activate
	if (m_drawTextures)
	{
		gl::setMatricesWindow(getWindowSize());
		gl::setViewport(getWindowBounds());
		gl::enableAlphaBlending();
		
		gl::disableDepthRead();
		gl::clear(ColorA(.0f, .0f, .0f, 1.0f));
		gl::color(ColorA(1.f, 1.f, 1.f, 1.f));
		
		m_clientSyphon.bind();
		gl::TextureRef sytex;// = m_clientSyphon.getTexture();
		sytex->bind(0);
		
		m_shdrDbg.bind();
		m_shdrDbg.uniform("dbgSy", 0);
		
		gl::pushMatrices();
		gl::draw(sytex, Rectf(0.0, 0.0, PARTICLES, PARTICLES));

		gl::popMatrices();
		
		sytex->unbind();
		m_clientSyphon.unbind();
		m_shdrDbg.unbind();
		
		// kinect image
		
		if (m_depthTex)
		{
			gl::draw(m_depthTex);
		}
		
	}
	// the actual particle system drawing
	else
	{
		m_fboSy.bindFramebuffer();
		gl::setMatricesWindow(m_fboSy.getSize());
		gl::setViewport(m_fboSy.getBounds());
		GLenum bfrs[1] = {GL_COLOR_ATTACHMENT0_EXT};
		glDrawBuffers(1, bfrs);
		
		gl::clear(ColorA(.0f, .0f, .0f, 1.0f));
		gl::enableAlphaBlending();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		gl::disableDepthRead();
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		
		m_fbo[m_bufferIn].bindTexture(0, 0);
		m_fbo[m_bufferIn].bindTexture(1, 1);
		m_fbo[m_bufferIn].bindTexture(2, 2);
		
		m_texSprite.bind(3);
		
		m_shdrPos.bind();
		m_shdrPos.uniform("texPos", 0);
		m_shdrPos.uniform("texVel", 1);
		m_shdrPos.uniform("texInf", 2);
		m_shdrPos.uniform("texSprite", 3);
		m_shdrPos.uniform("texSy", 4);
		m_shdrPos.uniform("scale", (float)PARTICLES);
		m_shdrPos.uniform("partSize", m_parts_size);
		m_shdrPos.uniform("numParts", m_parts_numparts);
		m_shdrPos.uniform("rotSpeed", m_parts_rotspeed);
		
		gl::color(ColorA(1.f, 1.f, 1.f, 1.f));
		gl::pushMatrices();
		
		gl::scale((float)SYWIDTH / (float)PARTICLES, (float)SYHEIGHT/(float)PARTICLES, 1.0f);
		
		gl::draw(m_vbo);
		
		gl::popMatrices();
		
		m_shdrPos.unbind();
		m_texSprite.unbind();
		m_fbo[m_bufferIn].unbindTexture();

		gl::disableAlphaBlending();
		
		
		m_fboSy.unbindFramebuffer();
		gl::draw(m_fboSy.getTexture());
		
		
		*m_texSyRef = m_fboSy.getTexture();
		m_srvSyphon.publishTexture(m_texSyRef);
	
	}
		
	// we could uncomment this to show the FPS, but drawing the text itself costs some performance
//	drawText();
	
	if (getElapsedFrames() % 60 == 0)
		console() << "FPS: " << getAverageFps() << std::endl;
	
}

void VideoToParticles::prepareSettings(Settings *settings)
{
	settings->setWindowSize(WIDTH,HEIGHT);
    settings->setFrameRate(30.0f);
	
}

void VideoToParticles::drawText()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	TextLayout layout;
	layout.setFont( Font( "Arial-BoldMT", 14 ) );
	layout.setColor( Color( 1.0f, 1.0f, 1.0f ) );
	layout.addLine( "1 Million Particles" );
	layout.addLine( " " );
	
	layout.setColor( Color( 0.9f, 0.9f, 0.9f ) );
	layout.setFont( Font( "ArialMT", 12 ) );
	
	layout.addLine("F - switch to fullscreen");
	layout.addLine("t - draw textures");
	
	char fps[50];
	sprintf(fps, "FPS: %.2f", getAverageFps());
    
	char particleCount[50];
	sprintf(particleCount, "Particles: %d", PARTICLES*PARTICLES);
	
	layout.addLine(fps);
	layout.addLine(particleCount);
    
    glEnable( GL_TEXTURE_2D );
	gl::draw(layout.render(true), Vec2f(getWindowWidth() - 150,25));
	glDisable( GL_TEXTURE_2D );
}

CINDER_APP_NATIVE( VideoToParticles, RendererGl )
