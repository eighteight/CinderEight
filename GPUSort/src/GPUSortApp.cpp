#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GPUSortApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    gl::BatchRef mBatch;
    
    gl::GlslProgRef mGlsl;
};

void GPUSortApp::setup()
{
    mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("transitionSort.vs")).fragment(loadAsset("transitionSort.fs")));
    
    mBatch = gl::Batch::create(geom::Cylinder().height(10.0f).radius(5.0f).subdivisionsAxis(60), gl::getStockShader(gl::ShaderDef().color()));
}

void GPUSortApp::mouseDown( MouseEvent event )
{
}

void GPUSortApp::update()
{
}

void GPUSortApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( GPUSortApp, RendererGl )
