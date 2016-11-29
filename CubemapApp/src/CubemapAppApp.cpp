#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CubeMapApp : public App {
public:
    static void prepare( Settings *settings );
    
    void setup() override;
    void update() override;
    void draw() override;
    
    void render();
    
private:
    gl::BatchRef  mTorus;
};

void CubeMapApp::prepare( Settings * settings )
{
    settings->setWindowSize( 800, 600 );
    settings->setResizable( false );
}

void CubeMapApp::setup()
{
    auto mesh = gl::VboMesh::create( geom::Torus().colors().radius( 1.0f, 0.95f ).subdivisionsAxis( 60.0f ).subdivisionsHeight( 60.0f ) );
    auto shader = gl::getStockShader( gl::ShaderDef().color() );
    mTorus = gl::Batch::create( mesh, shader );
}

void CubeMapApp::update()
{
}

void CubeMapApp::draw()
{
    gl::clear( Color::gray( 0.25f ) );
    
    gl::ScopedDepth depth( true, true );
    
    gl::ScopedColor color( 1, 1, 1 );
    gl::ScopedMatrices mat;
    
    // Left
    {
        gl::ScopedViewport viewport( ivec2( 0, 200 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( -1, 0, 0 ), vec3( 0, 1, 0 ) );
        gl::setMatrices( cam );
        
        render();
    }
    
    // Front
    {
        gl::ScopedViewport viewport( ivec2( 200, 200 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( 0, 0, -1 ), vec3( 0, 1, 0 ) );
        gl::setMatrices( cam );
        
        render();
    }
    
    // Right
    {
        gl::ScopedViewport viewport( ivec2( 400, 200 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( 1, 0, 0 ), vec3( 0, 1, 0 ) );
        gl::setMatrices( cam );
        
        render();
    }
    
    // Back
    {
        gl::ScopedViewport viewport( ivec2( 600, 200 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( 0, 0, 1 ), vec3( 0, 1, 0 ) );
        gl::setMatrices( cam );
        
        render();
    }
    
    // Top
    {
        gl::ScopedViewport viewport( ivec2( 200, 400 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( 0, 1, 0 ), vec3( 0, 0, 1 ) );
        gl::setMatrices( cam );
        
        render();
    }
    
    // Bottom
    {
        gl::ScopedViewport viewport( ivec2( 200, 0 ), ivec2( 200, 200 ) );
        
        CameraPersp cam;
        cam.setPerspective( 90.0f, 1.0f, 0.01f, 100.0f );
        cam.lookAt( vec3( 0 ), vec3( 0, -1, 0 ), vec3( 0, 0, -1 ) );
        gl::setMatrices( cam );
        
        render();
    }
}

void CubeMapApp::render()
{
    // Draw a cube. It will show where the faces of our map are.
    gl::drawStrokedCube( vec3( 0 ), vec3( 1 ) );
    
    // Draw several torii.
    Rand::randSeed( 12345678 );
    for(int i = 0; i < 100; ++i) {
        auto transform = glm::translate( Rand::randFloat( 1.2f, 3.6f ) * Rand::randVec3() );
        transform *= glm::rotate( (float)getElapsedSeconds() * Rand::randFloat( 0.5f, 2.0f ), Rand::randVec3() );
        
        gl::setModelMatrix( transform );
        mTorus->draw();
    }
    
}

CINDER_APP( CubeMapApp, RendererGl( RendererGl::Options().msaa( 4 ) ), &CubeMapApp::prepare )