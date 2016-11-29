#include "Resources.h"

#include "cinder/ObjLoader.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Arcball.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Sphere.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

#include <stdio.h>

using namespace ci;
using namespace ci::app;
using namespace std;

#include <list>
using std::list;

bool gDebug = false;

class CityTravelApp : public AppBasic {
 public:
	void	setup();
   	void	update();
	void	resize();

	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );

	void	frameCurrentObject();
	void	draw();
    void    updateCamPosition();
    void    setLocations();
    
    static const int VERTICES_X = 250, VERTICES_Z = 50;
	
	Arcball			mArcball;
	MayaCamUI		mMayaCam;
	TriMesh			mMesh;
	gl::VboMesh		mVBO, mVboMesh2;
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    
    vector<Vec3f> myLocations; 
    vector<Vec3f> myLookAts;
    float startTime;
    int a, b, c;
    vector<int> myPoints;
    Vec3f cubicInterpolate(cinder::Vec3f &p0, cinder::Vec3f &p1, cinder::Vec3f &p2, cinder::Vec3f &p3, float t);
    bool programCam;
    
    void setupVertices();
};

void CityTravelApp::setup(){
     setLocations();
	ObjLoader loader( (DataSourceRef)loadResource( RES_CUBE_OBJ ) );
	loader.load( &mMesh );
	mVBO = gl::VboMesh( mMesh );
	
	mTexture = gl::Texture( loadImage( loadResource( RES_IMAGE ) ) );
	mShader = gl::GlslProg( loadResource( RES_SHADER_VERT ), loadResource( RES_SHADER_FRAG ) );
    
	CameraPersp initialCam;
    //initialCam.setEyePoint(myLocations[0]);
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 100000 );

	mMayaCam.setCurrentCam( initialCam );
    
	mTexture.bind();
	mShader.bind();
	mShader.uniform( "tex0", 0 );

    programCam = false;
}

void CityTravelApp::setupVertices()
{
	// setup the parameters of the Vbo
	int totalVertices = VERTICES_X * VERTICES_Z;
	int totalQuads = ( VERTICES_X - 1 ) * ( VERTICES_Z - 1 );
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	layout.setDynamicPositions();
	layout.setStaticTexCoords2d();
	mVBO = gl::VboMesh( totalVertices, totalQuads * 4, layout, GL_QUADS );
	
	// buffer our static data - the texcoords and the indices
	vector<uint32_t> indices;
	vector<Vec2f> texCoords;
	for( int x = 0; x < VERTICES_X; ++x ) {
		for( int z = 0; z < VERTICES_Z; ++z ) {
			// create a quad for each vertex, except for along the bottom and right edges
			if( ( x + 1 < VERTICES_X ) && ( z + 1 < VERTICES_Z ) ) {
				indices.push_back( (x+0) * VERTICES_Z + (z+0) );
				indices.push_back( (x+1) * VERTICES_Z + (z+0) );
				indices.push_back( (x+1) * VERTICES_Z + (z+1) );
				indices.push_back( (x+0) * VERTICES_Z + (z+1) );
			}
			// the texture coordinates are mapped to [0,1.0)
			texCoords.push_back( Vec2f( x / (float)VERTICES_X, z / (float)VERTICES_Z ) );
		}
	}
	
	mVBO.bufferIndices( indices );
	mVBO.bufferTexCoords2d( 0, texCoords );
	
	// make a second Vbo that uses the statics from the first
	mVboMesh2 = gl::VboMesh( totalVertices, totalQuads * 4, mVBO.getLayout(), GL_QUADS, &mVBO.getIndexVbo(), &mVBO.getStaticVbo(), NULL );
	mVboMesh2.setTexCoordOffset( 0, mVBO.getTexCoordOffset( 0 ) );
	
	mTexture = gl::Texture( loadImage( loadResource( RES_IMAGE ) ) );
}

void CityTravelApp::resize( )
{
	App::resize( );
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( Vec2f( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( 150 );
}

void CityTravelApp::updateCamPosition(){
    if (!programCam) return;
    float elapsedTime = getElapsedSeconds() - startTime;
    
    float t = elapsedTime / 10.0f; // 2 seconds per section
    int index = floor(t);
    
    //if (index>myPoints.size()) return;
    
    // use only the fractional part of 't', so it lies between 0 and 1 and
    //  we can use it for interpolation
    t -= index;
    
    int p = (int)(elapsedTime / 10.0f);
    
#define BOUNDS(pp) { if (pp < 0) pp = 0; 	else if (pp >= (int)myPoints.size()-1) pp = myPoints.size() - 1; }
    
    int p0 = p - 1;     BOUNDS(p0);
    int p1 = p;         BOUNDS(p1);
    int p2 = p + 1;     BOUNDS(p2);
    int p3 = p + 2;     BOUNDS(p3);
    
    Vec3f location = cubicInterpolate(myLocations[p0], myLocations[p1], myLocations[p2], myLocations[p3], t);
    
    //cout<< "Loc: "<<location<<" "<<index<<" "<<p0<<" "<<p1<< " "<<p2<<" "<<p3<<" "<<t<<endl;
    cinder::Vec3f lookat = cubicInterpolate(myLookAts[p0], myLookAts[p1], myLookAts[p2], myLookAts[p3], t);                 
    CameraPersp cam = mMayaCam.getCamera();
    cam.setEyePoint( location );
    //cam.setCenterOfInterestPoint( lookat );
    mMayaCam.setCurrentCam( cam );
    
}

void CityTravelApp::mouseDown( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDown( event.getPos() );
	else
		mArcball.mouseDown( event.getPos() );
    
    std::cout<<"CAM:  "<<mMayaCam.getCamera().getEyePoint()<<" "<<mMayaCam.getCamera().getCenterOfInterestPoint()<<std::endl;

}

void CityTravelApp::mouseDrag( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	else
		mArcball.mouseDrag( event.getPos() );
    
    std::cout<<"CAM:  "<<mMayaCam.getCamera().getEyePoint()<<" "<<mMayaCam.getCamera().getCenterOfInterestPoint()<<std::endl;

}

void CityTravelApp::frameCurrentObject()
{
	Sphere boundingSphere = Sphere::calculateBoundingSphere( mMesh.getVertices() );
	
	mMayaCam.setCurrentCam( mMayaCam.getCamera().getFrameSphere( boundingSphere, 100 ) );
}

void CityTravelApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'o' ) {
		std::string path = getOpenFilePath().string();
		if( ! path.empty() ) {
			ObjLoader loader( (DataSourceRef)loadFile( path ) );
			loader.load( &mMesh, true );
			mVBO = gl::VboMesh( mMesh );
			console() << "Total verts: " << mMesh.getVertices().size() << std::endl;
		}
	}
	else if( event.getChar() == 's' ) {
		std::string path = getSaveFilePath( "output.trimesh" ).string();
		if( ! path.empty() ) {
			console() << "Saving to " << path;
			mMesh.write( writeFile( path ) );
		}
	}
	else if( event.getChar() == 'j' ) {
		std::string path = getSaveFilePath( "output.obj" ).string();
		if( ! path.empty() ) {
			console() << "Saving to " << path;
			ObjLoader::write( writeFile( path ), mMesh );
		}	
	}
	else if( event.getChar() == 'f' ) {
		frameCurrentObject();
	}
	else if( event.getChar() == 'p' ) {
        CameraPersp cam = mMayaCam.getCamera();
        cam.setEyePoint(myLocations[0]);
        mMayaCam.setCurrentCam(cam);
        startTime = getElapsedSeconds();
		programCam = ! programCam;
	}
}

void CityTravelApp::update(){
    updateCamPosition();
}

Vec3f CityTravelApp::cubicInterpolate(Vec3f &p0, Vec3f &p1, Vec3f &p2, Vec3f &p3, float t)
{
    //return Vec3f(1.0f,1.0f,1.0f);
    return p1 + 0.5f*t*(p2-p0 + t*(2*p0-5*p1+4*p2-p3 + t*(3*(p1-p2)+p3-p0)));
}
void CityTravelApp::draw()
{
	gl::enableDepthWrite();
	gl::enableDepthRead();
    //black	
    gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
    //orange	
    gl::clear( Color( 0.97f, 0.5f, 0.1f ) );
	//blue
    //gl::clear( Color( 0.0f, 0.1f, 1.0f ) );
	glDisable( GL_CULL_FACE );

	gl::setMatrices( mMayaCam.getCamera() );

/*	Sphere boundingSphere = Sphere::calculateBoundingSphere( mMesh.getVertices() );
	glColor3f( 1.0f, 1.0f, 1.0f );
	gl::disableDepthWrite();
	mTexture->disable();
	mShader.unbind();
	gl::draw( boundingSphere, 30 );
	gl::enableDepthWrite();
*/
	mShader.bind();
	gl::pushMatrices();
		gl::rotate( mArcball.getQuat() );
		gl::draw( mVBO );
	gl::popMatrices();
}

void CityTravelApp::setLocations(){
    startTime = getElapsedSeconds();
    myLocations.push_back(Vec3f(47202.7, 35402.1, 47202.7));
    myLocations.push_back(Vec3f(8720.7, 6540.1, 8720.7));
    myLocations.push_back(Vec3f(4720.7, 3540.1, 4720.7));
    myLocations.push_back(Vec3f(872.7, 854.1, 872.7));
    myLocations.push_back(Vec3f(472.7, 354.1, 472.7));
    myLocations.push_back(Vec3f(272.7, 154.1, 272.7));
    myLocations.push_back(Vec3f(172.7, 104.1, 172.7));
    myLocations.push_back(Vec3f(100.7, 94.1, 102.7));
    myLocations.push_back(Vec3f(80.7, 64.1, 80.7));
    myLocations.push_back(Vec3f(70.7, 54.1, 60.7));
    myLocations.push_back(Vec3f(28.7, 21.1, 28.7));
    myLocations.push_back(Vec3f(28.7, 21.1, 28.7));
    myLocations.push_back(Vec3f(28.7, 21.1, 28.7));
    myLocations.push_back(Vec3f(28.7, 21.1, 28.7));
    myLocations.push_back(Vec3f(28.7, 21.1, 28.7));
    
    myLookAts.push_back(Vec3f(0.0f, 0.0f, 0.0f));
    myLookAts.push_back(Vec3f(1.0f, 1.0f, 1.0f));
    myLookAts.push_back(Vec3f(10.0f, 10.0f, 10.0f));
    myLookAts.push_back(Vec3f(20.0f, 20.0f, 20.0f));
    myLookAts.push_back(Vec3f(20.0f, 20.0f, 20.0f));
    myLookAts.push_back(Vec3f(200.0f, 200.0f, 200.0f));
    myLookAts.push_back(Vec3f(200.0f, 200.0f, 200.0f));
    
    myPoints.push_back(0);
    myPoints.push_back(1);
    myPoints.push_back(2);
    myPoints.push_back(3);
    myPoints.push_back(4);
    myPoints.push_back(5);
    myPoints.push_back(6);
    myPoints.push_back(7);
    myPoints.push_back(8);
    myPoints.push_back(9);
    myPoints.push_back(10);
    myPoints.push_back(11);
    myPoints.push_back(12);
    myPoints.push_back(13);
    myPoints.push_back(14);
}


CINDER_APP_BASIC( CityTravelApp, RendererGl )