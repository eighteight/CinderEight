#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class glNextExampleApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    CameraPersp mCam;
    gl::BatchRef mBatch;
    
    gl::GlslProgRef mGlsl;
};

void glNextExampleApp::setup()
{
    mCam.setPerspective(60, getWindowAspectRatio(), .01, 1000);
    mCam.lookAt(vec3(0,10,20), vec3(0,0,0));
    
    //mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Basic.vert")).fragment(loadAsset("Basic.frag")));
    mBatch = gl::Batch::create(geom::Cylinder().height(10.0f).radius(5.0f).subdivisionsAxis(60), gl::getStockShader(gl::ShaderDef().color()));
}

void glNextExampleApp::mouseDown( MouseEvent event )
{
}

void glNextExampleApp::update()
{
}

void glNextExampleApp::draw()
{
    static float rotation = 0.0f;
	gl::clear( Color( 0, 0, 0 ) );
    gl::setMatrices(mCam);
    gl::color(Color(1,0,0));
    gl::multViewMatrix(ci::rotate(rotation += 0.01, vec3(0,1,0)));
//    {
//        gl::ScopedModelMatrix scopeModel;
//        gl::multModelMatrix(ci::translate(vec3(-1,0,0)));
//        gl::draw(geom::Teapot());
//    }
    {
        gl::ScopedModelMatrix scopeModel;
        gl::multModelMatrix(ci::translate(vec3(1,0,0)));
        mBatch->draw();
    }
}

CINDER_APP_NATIVE( glNextExampleApp, RendererGl )
