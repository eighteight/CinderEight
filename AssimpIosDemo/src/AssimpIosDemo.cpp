#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Camera.h"

#include "AssimpLoader.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"
#include "Renderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const pair<float,float> CAMERA_Y_RANGE( 32, 80 );

class AssimpApp : public App {
public:
	void setup() override;
	void touchesMoved( TouchEvent event ) override;
	void update() override;
	void draw() override;
    void resize() override;
	
	model::SkeletalMeshRef		mSkeletalMesh;
	float						mTouchHorizontalPos;
	float						mRotationRadius;
    CameraPersp                 mCam;
};

void AssimpApp::setup()
{
	using namespace model;
	mSkeletalMesh = SkeletalMesh::create( AssimpLoader( loadAsset( "TurbochiFromXSI.dae" ) ) );
	mRotationRadius = 20.0f;
	mTouchHorizontalPos = 0;
    
    mCam.lookAt( vec3( 0, CAMERA_Y_RANGE.first, 0 ), vec3( 0 ) );
	gl::enableDepthRead();
}

void AssimpApp::resize()
{
    mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
    gl::setMatrices( mCam );
}

void AssimpApp::touchesMoved( TouchEvent event )
{
	auto touches = event.getTouches();
	mTouchHorizontalPos = float( toPixels( touches[0].getX() ) );
}

void AssimpApp::update()
{
	if( mSkeletalMesh->getSkeleton() ) {
		float time = mSkeletalMesh->getAnimDuration() * mTouchHorizontalPos / toPixels(getWindowWidth());
		mSkeletalMesh->setPose( time );
	}
	
    model::Renderer::getLight()->position.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
    model::Renderer::getLight()->position.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void AssimpApp::draw()
{
	// clear out the window with black
	gl::clear( Color(0.45f, 0.5f, 0.55f) );
    gl::setMatrices( mCam );
	
	model::Renderer::draw( mSkeletalMesh );
}

CINDER_APP( AssimpApp, RendererGl )
