#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;

using std::vector;


/*** This sample demonstrates the Vbo class by creating a simple grid mesh with a texture mapped onto it.
 * The mesh has static indices and texture coordinates, but its vertex positions are dynamic.
 * It also creates a second mesh which shares static and index buffers, but has its own dynamic buffer ***/

class VboSampleApp : public AppBasic {
 public:
	void setup();
	void update();
	void draw();

	static const int VERTICES_X = 250, VERTICES_Z = 50;

	gl::VboMesh		mesh, mVboMesh2;
	gl::Texture		mTexture;
	CameraPersp		mCamera;
    std::vector<Vec3f> positions;
};

void VboSampleApp::setup()
{
    gl::VboMesh::Layout layout;
    layout.setStaticIndices();
    layout.setDynamicColorsRGBA();
    layout.setStaticPositions();
    
    int vertCount = 24;
    int quadCount = 1;
    mesh = gl::VboMesh(vertCount, quadCount * 4, layout, GL_QUADS);
    
    
    
    std::vector<uint32_t> indices;
    int i=0;
    while(i < 24){
        indices.push_back(i);
        i++;
    }
    
    mesh.bufferIndices(indices);
    
    positions.push_back(Vec3f(100, 200, 1));
    positions.push_back(Vec3f( 200, 200, 1));
    positions.push_back(Vec3f( 200, 100, 1));
    positions.push_back(Vec3f(100, 100, 1));
    
//    positions.push_back(Vec3f( 200, 200, 1));
//    positions.push_back(Vec3f( 200, 200, 100));
//    positions.push_back(Vec3f( 200, 100, 100));
//    positions.push_back(Vec3f( 200, 100, 1));
//    
//    positions.push_back(Vec3f( 200, 200, 100));
//    positions.push_back(Vec3f(100, 200, 100));
//    positions.push_back(Vec3f(100, 100, 100));
//    positions.push_back(Vec3f( 200, 100, 100));
//    
//    positions.push_back(Vec3f(100, 200, 100));
//    positions.push_back(Vec3f(100, 200, 1));
//    positions.push_back(Vec3f(100, 100, 1));
//    positions.push_back(Vec3f(100, 100, 100));
//    
//    positions.push_back(Vec3f(100, 200, 100));
//    positions.push_back(Vec3f( 200, 200, 100));
//    positions.push_back(Vec3f( 200, 200, 1));
//    positions.push_back(Vec3f(100, 200, 1));
//    
//    positions.push_back(Vec3f(100, 100, 100));
//    positions.push_back(Vec3f( 200, 100, 100));
//    positions.push_back(Vec3f( 200, 100, 1));
//    positions.push_back(Vec3f(100, 100, 1));
    
    // now we can buffer positions
    mesh.bufferPositions(positions);
}

void VboSampleApp::update()
{
    float g = sin(getElapsedSeconds());
    float b = cos(getElapsedSeconds());
    gl::VboMesh::VertexIter iter = mesh.mapVertexBuffer();
    for( int x = 0; x < 24; ++x ) {
        //positions.at(x) *= 1.001;
        iter.setColorRGBA( ColorA( 1 - (g+b/3), g, b, 0.5) );
        ++iter;
    }
}

void VboSampleApp::draw()
{
	// this pair of lines is the standard way to clear the screen in OpenGL
	gl::clear( Color( 0.15f, 0.15f, 0.15f ) );

	//gl::scale( Vec3f( 10, 10, 10 ) );
	//mTexture.enableAndBind();
	gl::draw( mesh );
	//gl::draw( mVboMesh2 );
}


CINDER_APP_BASIC( VboSampleApp, RendererGl )
