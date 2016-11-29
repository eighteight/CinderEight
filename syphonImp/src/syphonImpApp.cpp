#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinderSyphon.h"

#include <list>

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 800
#define HEIGHT 600

class spiral {
	
public:
	spiral(){
		limit = 10.0f;
		resolution = 0.1f;
	}
	
	void set(float _limit, float _resolution){
		limit = _limit;
		resolution = _resolution;
	}
	
	void calc(){
		Vec2f here;
		
		for(float t = 0.0f; t <= limit; t += resolution){
			here.x = t * cos(t);
			here.y = t * sin(t);
			mSpiral.push_back(here);
		}
	}
	
	void draw(){
		
		glBegin(GL_LINES);
		
		for(vector<Vec2f>::iterator iter = mSpiral.begin(); iter != mSpiral.end(); ++iter){
			if(iter != mSpiral.begin()){
				glVertex2f((iter-1)->x, (iter-1)->y);
				glVertex2f(iter->x, iter->y);
			}
		}
		glEnd();
	}
	
	vector<Vec2f> mSpiral;
	float limit;
	float resolution;
};

// We'll create a new Cinder Application by deriving from the AppBasic class
class syphonImpApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event );
	void mouseUp( MouseEvent event );
	void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void setup();
	void update();
	void draw();
	
	void randomizeSurface(Surface* inputSurface);
	
	gl::Texture mTex;
	Surface mSurface;
	spiral archimedes;
	float mRot;
	
	syphonServer mScreenSyphon;
	//syphonServer mTextureSyphon;
	
	//syphonClient mClientSyphon;
};

void syphonImpApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(WIDTH,HEIGHT);
	settings->setFrameRate(60.f);
}

void syphonImpApp::setup()
{
	mTex = gl::Texture(200, 100);
	mSurface = Surface8u(200, 100, false);
	randomizeSurface(&mSurface);
	mTex.update(mSurface);
	
	archimedes.set(100.0f, 0.6f);
	archimedes.calc();
	mRot = 0.f;
	
	mScreenSyphon.setName("Cinder Screen");
	//mTextureSyphon.setName("Cinder Texture");
	
//	mClientSyphon.setup();
    
//    mClientSyphon.setApplicationName("Simple Server");
//    mClientSyphon.setServerName("");
}

void syphonImpApp::update()
{
	randomizeSurface(&mSurface);
	mTex.update(mSurface);
	mRot += 0.2f;
}

void syphonImpApp::draw()
{
	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
	
	gl::pushModelView();
	gl::translate(Vec2f(getWindowWidth()/2, getWindowHeight()/2));
	gl::rotate(mRot);
	gl::scale(Vec3f(4.f, 4.f, 1.f));
	gl::color(ColorA(1.f, 0.f, 0.f, 1.f));
	archimedes.draw();
	gl::popModelView();
	
	if(mTex){
		gl::color(ColorA(1.f, 1.f, 1.f, 1.f));
		gl::draw(mTex);
	}
	
	//mScreenSyphon.publishScreen();
	mTextureSyphon.publishTexture(&mTex);
	
	//mClientSyphon.draw(0, 0);
}

void syphonImpApp::randomizeSurface(Surface* inputSurface)
{
	Surface::Iter inputIter( inputSurface->getIter() );
	while( inputIter.line() ) {
		while( inputIter.pixel() ) {
			inputIter.r() = Rand::randInt(0, 255);
			inputIter.g() = Rand::randInt(0, 255);
			inputIter.b() = Rand::randInt(0, 255);
		}
	}
}

void syphonImpApp::keyDown( KeyEvent event )
{
	//
}

void syphonImpApp::mouseDown( MouseEvent event )
{
	//
}

void syphonImpApp::mouseUp( MouseEvent event )
{
	//
}

void syphonImpApp::mouseMove( MouseEvent event )
{
	//
}

void syphonImpApp::mouseDrag( MouseEvent event )
{
	//
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( syphonImpApp, RendererGl )

