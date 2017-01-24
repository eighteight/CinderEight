#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/GeomIO.h"
#include "cinder/ImageIo.h"
#include "cinder/CameraUi.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;

using std::vector;

class VboParticlesApp : public App {
public:
    void	setup() override;
    void	update() override;
    void	draw() override;
    
private:
    gl::VboMeshRef	mVboMesh;
    
    CameraPersp		mCamera;
    CameraUi		mCamUi;
};

void VboParticlesApp::setup()
{
    gl::VboMesh::Layout layout;
    layout.usage( GL_DYNAMIC_DRAW )
    .attrib( geom::POSITION, 3 );
    
    auto plane = geom::Plane().size( vec2( 20, 20 ) ).subdivisions( ivec2( 50, 50 ) );
    auto triMesh = TriMesh::create( plane );
    mVboMesh = gl::VboMesh::create( triMesh->getNumVertices(), GL_POINTS, { layout } );
    mVboMesh->bufferAttrib( geom::POSITION,
                           triMesh->getNumVertices() * sizeof( vec3 ),
                           triMesh->getPositions< 3 >() );
    
    mCamUi = CameraUi( &mCamera, getWindow() );
}

void VboParticlesApp::update()
{
    float offset = getElapsedSeconds() * 4.0f;
    
    // Dynmaically generate our new positions based on a sin(x) + cos(z) wave
    // We set 'orphanExisting' to false so that we can also read from the position buffer, though keep
    // in mind that this isn't the most efficient way to do cpu-side updates. Consider using VboMesh::bufferAttrib() as well.
    auto mappedPosAttrib = mVboMesh->mapAttrib3f( geom::Attrib::POSITION, false );
    for( int i = 0; i < mVboMesh->getNumVertices(); i++ ) {
        vec3 &pos = *mappedPosAttrib;
        mappedPosAttrib->y = sinf( pos.x * 1.1467f + offset ) * 0.323f + cosf( pos.z * 0.7325f + offset ) * 0.431f;
        ++mappedPosAttrib;
    }
    mappedPosAttrib.unmap();
}

void VboParticlesApp::draw()
{
    gl::clear( Color( 0.15f, 0.15f, 0.15f ) );
    
    gl::setMatrices( mCamera );
    
    gl::ScopedGlslProg glslScope( gl::getStockShader( gl::ShaderDef() ) );
    
    gl::draw( mVboMesh );
}

CINDER_APP( VboParticlesApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
