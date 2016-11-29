#include "cinder/app/App.h"

#include "cinder/app/RendererGl.h"

#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"


using namespace ci;

using namespace ci::app;

using namespace std;



class Day14App : public App {
    
public:
    
    void setup() override;
    
    void mouseDown( MouseEvent event ) override;
    
    void update() override;
    
    void draw() override;
    
    
    
    void saveGif();
    
    
    
    CameraPersp     mCam;
    CameraUi		mCamUi;
    
    
    gl::VboRef           mVbo;
    
    gl::VboMeshRef       mTriangle;
    
    gl::VaoRef           mVao;
    
    gl::BatchRef         mBatch;
    
    gl::GlslProgRef     mGlsl;
    
    vector < vec3 >        mPositions;     //VERTEX POSITIONS
    vector < vec3 >        mNormals;
    vector < uint32_t >    mIndices;       //INDICES
    
};



void Day14App::setup()
{
    mCam.lookAt( vec3( 0, 0, 10 ), vec3( 0 ) );
    mCamUi = CameraUi( &mCam, getWindow() );
    
    mPositions.push_back( vec3( -0.5, -0.5, 0.0 ) );
    mPositions.push_back( vec3( 0.5, -0.5, 0.0 ) );
    mPositions.push_back( vec3( 0.0, 0.5, 0.0 ) );
    
    auto normal = vec3( 0, 0, 1 );
    mNormals.push_back( normal );
    mNormals.push_back( normal );
    mNormals.push_back( normal );
    
    mIndices.push_back( 0 );
    mIndices.push_back( 2 );
    mIndices.push_back( 1 );
    
    vector < gl::VboMesh::Layout > bufferLayout = {
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::POSITION , 3 ),
        gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::Attrib::NORMAL , 3 )
    };
    
    mTriangle = gl::VboMesh::create( mPositions.size(), GL_TRIANGLES, bufferLayout, mIndices.size(), GL_UNSIGNED_INT );
    
    mTriangle->bufferAttrib( geom::Attrib::POSITION, mPositions );
    mTriangle->bufferAttrib( geom::Attrib::NORMAL, mNormals );
    mTriangle->bufferIndices( mIndices.size() * sizeof( uint32_t ), mIndices.data() );
    
    mGlsl = gl::getStockShader( gl::ShaderDef().lambert().color() );
    mBatch = gl::Batch::create( mTriangle, mGlsl );
}



void Day14App::mouseDown( MouseEvent event )

{
    
}



void Day14App::update()
{
    
}



void Day14App::draw()
{
    gl::clear( Color::gray( 0.2f ) );
    
    
    gl::setMatrices( mCam );
    
    gl::color( Color( 1, 1, 1 ) );
    
    mBatch->draw();
}



CINDER_APP( Day14App, RendererGl( RendererGl::Options().msaa( 16 ) ),
           
           [&]( App::Settings *settings ) {
               
               settings->setHighDensityDisplayEnabled();
               
               settings->setWindowSize( 500, 500 );
               
           } )