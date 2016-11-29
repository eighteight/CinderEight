#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Lighting_ShadersApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    gl::GlslProgRef mFlatShader;
    gl::BatchRef mTeapot;
    CameraPersp mCamera;
    vec4 mLightPos;
};

void Lighting_ShadersApp::setup()
{
    
    mCamera.setPerspective(60, getWindowAspectRatio(), 0.1f, 1000);
    mCamera.lookAt(vec3(0,0,5), vec3(0,0,0));

    mFlatShader = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Flat.vert")).fragment(loadAsset("Flat.frag")));

    mTeapot = gl::Batch::create(geom::Teapot().enable(geom::Attrib::NORMAL), mFlatShader);
    
    mLightPos = vec4(5,5,5,1);
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
}

void Lighting_ShadersApp::mouseDown( MouseEvent event )
{
}

void Lighting_ShadersApp::update()
{
    mLightPos.x = sin(getElapsedFrames()) * mLightPos.x;
}

void Lighting_ShadersApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::setMatrices(mCamera);
    mTeapot->getGlslProg()->uniform("lightPosition", mLightPos);
    {
        gl::ScopedMatrices push;
        mTeapot->draw();
        
    }
}

CINDER_APP_NATIVE( Lighting_ShadersApp, RendererGl )
