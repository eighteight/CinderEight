#include "ParticleSystem.h"

#include "cinder/app/AppNative.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/ObjLoader.h"

#define WIDTH 100
#define HEIGHT 200

using namespace ci;
using namespace ci::app;

/*
   ALL CREDIT for this code goes to KYLE MCDONALD and his original OF version
   posted here http://www.openframeworks.cc/forum/viewtopic.php?f=12&t=2860
*/

// BPS stands for Binned Particle System
class bpsApp : public AppNative {
 public:	
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();
	
	void keyDown( KeyEvent event );
	void mouseDown( MouseEvent event);
	void mouseUp( MouseEvent event);
	void mouseDrag( MouseEvent event );
    
    void setupObj();
	
	Vec2i mouse;
	
	float timeStep;
	float lineOpacity, pointOpacity;
	float particleNeighborhood, particleRepulsion;
	float centerAttraction;
	
	int kParticles;
	ParticleSystem particleSystem;
	bool isMousePressed, slowMotion;
};

void bpsApp::prepareSettings( Settings *settings )
{
	settings->setFullScreen( true );
}
void bpsApp::setupObj(){
    TriMesh mMesh;
    ObjLoader loader( (DataSourceRef)loadResource( "albena1.obj" ), (DataSourceRef)loadResource( "albena1.mtl" ) );
    loader.load( &mMesh );
    size_t mCnt = 0;
    size_t	nTri =	mMesh.getNumTriangles();
    size_t total = HEIGHT * WIDTH;
    Vec3f a, b, c;
    float maxVelocity = .5;
    while (mCnt< total) {
        for (int i = 0; i < nTri && mCnt < total; i++){
            
            mMesh.getTriangleVertices(i, &a, &b, &c);
            
            float w1 = Rand::randFloat(0.0f, 1.0f);
            float w2 = Rand::randFloat(0.0f, 1.0f - w1);
            float w3 = 1.0f - w1 - w2;
            Vec3f posf = a*w1 + b*w2 + c*w3;
            Vec2i posi ((int)((posf.x+0.5)*(float)WIDTH), (int)((posf.y-0.5)*(float)HEIGHT));
            //cout<<posi<< " " << posf<<endl;
            float xv = Rand::randFloat(-maxVelocity, maxVelocity);
            float yv = Rand::randFloat(-maxVelocity, maxVelocity);
            Particle particle(posf.x, posf.y, xv, yv);
            particleSystem.add(particle);
           // mInitPos.setPixel( posi, ColorAf( posf.x+0.5, posf.y-0.5f, posf.z , 0.001*Rand::randFloat(0.2f, 1.0f) ) );
            mCnt ++;
        }
    }
  
}
void bpsApp::setup(){
	// this number describes how many bins are used
	// on my machine, 2 is the ideal number (2^2 = 4x4 pixel bins)
	// if this number is too high, binning is not effective
	// because the screen is not subdivided enough. if
	// it's too low, the bins take up so much memory as to
	// become inefficient.
	int binPower = 5;
	
	particleSystem.setup(getWindowWidth(), getWindowHeight(), binPower);
    setupObj();
	kParticles = 12;
	float padding = 0;
	float maxVelocity = .05;
//	for(int i = 0; i < kParticles * 1024; i++) {
//		float x = Rand::randFloat(padding, getWindowWidth() - padding);
//		float y = Rand::randFloat(padding, getWindowHeight() - padding);
//		float xv = Rand::randFloat(-maxVelocity, maxVelocity);
//		float yv = Rand::randFloat(-maxVelocity, maxVelocity);
//		Particle particle(x, y, xv, yv);
//		particleSystem.add(particle);
//	}
	
	timeStep = 1;
	lineOpacity = 0.12f;
	pointOpacity = 0.5f;
	slowMotion = true;
	particleNeighborhood = 14;
	particleRepulsion = 0.105;
	centerAttraction = .0005;
}

void bpsApp::update(){
	particleSystem.setTimeStep(timeStep);
}

void bpsApp::draw()
{
	gl::clear();
	gl::enableAdditiveBlending();
	glColor4f(1.0f, 1.0f, 1.0f, lineOpacity);
	
	particleSystem.setupForces();
	// apply per-particle forces
	glBegin(GL_LINES);
	for(int i = 0; i < particleSystem.size(); i++) {
		Particle& cur = particleSystem[i];
		// global force on other particles
		particleSystem.addRepulsionForce(cur, particleNeighborhood, particleRepulsion);
		// forces on this particle
		cur.bounceOffWalls(0, 0, getWindowWidth(), getWindowHeight());
		cur.addDampingForce();
	}
	glEnd();
	// single global forces
	particleSystem.addAttractionForce(getWindowWidth()/2, getWindowHeight()/2, getWindowWidth(), centerAttraction);
	if(isMousePressed)
		particleSystem.addRepulsionForce(mouse.x, mouse.y, 100, 10);
	particleSystem.update();
	glColor4f(1.0f, 1.0f, 1.0f, pointOpacity);
	particleSystem.draw();
	gl::disableAlphaBlending();
	
	glColor3f(1.0f, 1.0f, 1.0f);
	gl::drawString( toString( kParticles ) + "k particles", Vec2f(32.0f, 32.0f));
	gl::drawString( toString((int) getAverageFps()) + " fps", Vec2f(32.0f, 52.0f));
}


void bpsApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 's' ) {
		slowMotion = !slowMotion;
		if(slowMotion)
			timeStep = .05;
		else
			timeStep = 1;
	}
}

void bpsApp::mouseDown( MouseEvent event )
{
	isMousePressed = true;
	mouse = Vec2i(event.getPos());
}

void bpsApp::mouseUp( MouseEvent event )
{
	isMousePressed = false;
}

void bpsApp::mouseDrag( MouseEvent event )
{
	mouse = Vec2i(event.getPos());
}



// This line tells Cinder to actually create the application
CINDER_APP_NATIVE( bpsApp, RendererGl )
