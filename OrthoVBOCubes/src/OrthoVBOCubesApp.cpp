#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OrthoVBOCubesApp : public AppNative {
  public:
	void prepareSettings( Settings *settings ) override;
    void setup();
    void resize();
    void draw();
    
    cinder::CameraOrtho mCam;
    gl::VboMesh mCube;
    int mSteps = 19;
    int mMargin = 20;
    int mLength = 20;
};

void OrthoVBOCubesApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
    settings->enableMultiTouch( false );
    settings->setWindowSize(800, 800);
}

void OrthoVBOCubesApp::setup()
{
    // We can't use gl::drawCube since it uses triangle faces so we'll build
    // one using quads.
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setStaticPositions();
    
    int vertCount = 24;
    int quadCount = 6;
    mCube = gl::VboMesh(vertCount, quadCount * 4, layout, GL_QUADS);
    
    vector<uint32_t> indices;
    for (int i=0; i < vertCount; i++) {
        indices.push_back(i);
    }
    mCube.bufferIndices(indices);
    
    vector<Vec3f> positions;
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    
    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );
    
    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );
    
    positions.push_back( Vec3f( -0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5,  0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5,  0.5, -0.5 ) );
    
    positions.push_back( Vec3f( -0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5,  0.5 ) );
    positions.push_back( Vec3f(  0.5, -0.5, -0.5 ) );
    positions.push_back( Vec3f( -0.5, -0.5, -0.5 ) );
    mCube.bufferPositions(positions);
}

void OrthoVBOCubesApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    float perCube = min( width, height ) / mSteps;
    mLength = round( perCube * 0.55 );
    mMargin = round( perCube * 0.45 );
    
    mCam.setOrtho( 0, width, height, 0, -100, 100 );
    gl::setMatrices( mCam );
}

void OrthoVBOCubesApp::draw()
{
    float angleStep = 90 / (mSteps - 1);
    
    gl::clear( Color::white() );
    gl::color( Color::black() );
    
    gl::enableWireframe();
    
    gl::pushModelView();
    gl::translate( Vec2f( 1, 1 ) * mLength );
    
    for (int y = 0; y < mSteps; y++) {
        for (int x = 0; x < mSteps; x++) {
            gl::pushModelView();
            gl::translate( Vec2f( x, y ) * (mLength + mMargin) );
            gl::scale( Vec3f( mLength, mLength, mLength ));
            gl::rotate( Vec3f( angleStep * y, angleStep * x, 0 ));
            gl::draw( mCube );
            gl::popModelView();
        }
    }
    gl::disableWireframe();
    
    // Save a frame in the home directory.
    if (getElapsedFrames() == 1) {
        writeImage( getHomeDirectory() / "SquarePerspectiveAppOutput.png", copyWindowSurface() );
    }
    
    gl::popModelView();
}

CINDER_APP_NATIVE( OrthoVBOCubesApp, RendererGl )
