// About this: https://drewish.com/2014/08/23/using-cinder's-cameraortho-and-vbomesh-to-draw-cubes/
// Inspired by: http://onepointperspective.tumblr.com/post/7805032561/the-perspective-of-a-square
// Use with Cinder 0.9.x

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void prepareSettings( App::Settings *settings )
{
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize(800, 800);
}

class SquaresOtakuGangstaApp : public App {
public:
    void setup() override;
    void resize() override;
    void draw() override;
    
    cinder::CameraOrtho mCam;
    gl::VboMeshRef mCube;
    int mSteps = 19;
    int mMargin = 20;
    int mLength = 20;
};

void SquaresOtakuGangstaApp::setup()
{
    // I'm sure there's a simpler ordering for these points...
    //
    // *-X  7----3
    // |\   |\   |\
    // Y Z  | 6----2
    //      5-|--1 |
    //       \|   \|
    //        4----0
    vector<vec3> positions = {
        vec3( +0.5, +0.5, +0.5 ),
        vec3( +0.5, +0.5, -0.5 ),
        vec3( +0.5, -0.5, +0.5 ),
        vec3( +0.5, -0.5, -0.5 ),
        vec3( -0.5, +0.5, +0.5 ),
        vec3( -0.5, +0.5, -0.5 ),
        vec3( -0.5, -0.5, +0.5 ),
        vec3( -0.5, -0.5, -0.5 ),
    };
    
    // Lines by axis
    // X: 0-4, 1-5, 2-6, 3-7
    // Y: 0-2, 1-3, 4-6, 5-7
    // Z: 0-1, 2-3, 4-5, 6-7
    vector<uint> indices = {
        0, 4, 1, 5, 2, 6, 3, 7,
        0, 2, 1, 3, 4, 6, 5, 7,
        0, 1, 2, 3, 4, 5, 6, 7,
    };
    
    vector<gl::VboMesh::Layout> bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION, 3 ),
    };
    mCube = gl::VboMesh::create( positions.size(), GL_LINES, bufferLayout, indices.size(), GL_UNSIGNED_INT );
    mCube->bufferAttrib( geom::Attrib::POSITION, positions );
    mCube->bufferIndices( indices.size() * sizeof( uint ), indices.data() );
}

void SquaresOtakuGangstaApp::resize()
{
    int height = getWindowHeight();
    int width = getWindowWidth();
    float perCube = min( width, height ) / mSteps;
    mLength = round( perCube * 0.55 );
    mMargin = round( perCube * 0.45 );
    
    mCam.setOrtho( 0, width, height, 0, -100, 100 );
    gl::setMatrices( mCam );
}

void SquaresOtakuGangstaApp::draw()
{
    gl::ScopedModelMatrix outerModelScope;
    float radiansPerStep = M_PI_2 / (mSteps - 1);
    
    gl::clear( Color::white() );
    gl::color( Color::black() );
    
    gl::translate( vec2( mLength ) );
    
    for (int y = 0; y < mSteps; y++) {
        for (int x = 0; x < mSteps; x++) {
            gl::ScopedModelMatrix innerModelScope;
            
            gl::translate( vec2( x, y ) * vec2( mLength + mMargin ) );
            gl::scale( vec3( mLength, mLength, mLength ));
            gl::rotate( radiansPerStep * y, vec3( 1, 0, 0 ) );
            gl::rotate( radiansPerStep * x, vec3( 0, 1, 0 ) );
            
            gl::draw( mCube );
        }
    }
    
    // Save a frame in the home directory.
    if (getElapsedFrames() == 1) {
        writeImage( getHomeDirectory() / "SquarePerspectiveAppOutput.png", copyWindowSurface() );
    }
}

CINDER_APP( SquaresOtakuGangstaApp, RendererGl( RendererGl::Options().msaa( 16 ) ), prepareSettings )