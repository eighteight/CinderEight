#include "cinder/ImageIo.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/TriMesh.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Texture.h"
#include "cinder/ip/Hdr.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SmoothMeshApp : public AppBasic {
public:
	void setup();
	void update();
	void draw();
	
	// events
	void mouseDown( MouseEvent event );	
	void mouseDrag( MouseEvent event );	

	void keyDown( KeyEvent event );

	void resize( );
private:
	void enableLights();
	void disableLights();

	void setupShadowMap();
	void renderShadowMap();

	void setupMesh( uint32_t model = 3 );
private:
	bool			mDrawWireframe;
	bool			mDrawFlatShaded;
	bool			mDrawShadowMap;

	Vec3f			mLightPosition;

	TriMesh			mTriMesh;
	MayaCamUI		mMayaCam;

	gl::Fbo			mDepthFbo;
	gl::GlslProg	mShader;
	gl::Texture		mTexture;

	Matrix44f		mShadowMatrix;
	CameraPersp		mShadowCamera;
    void generateNormals(TriMesh);
};

void SmoothMeshApp::setup()
{
	mDrawWireframe = false;
	mDrawFlatShaded = false;
	mDrawShadowMap = false;

	// create the mesh
	setupMesh();

	// initialize shadow map
	setupShadowMap();

	// load the texture from the "assets" folder (easier than using resources)
	try {	mTexture = gl::Texture( loadImage( loadAsset("texture.jpg") ) );	}
	catch( const std::exception &e ) { console() << "Could not load texture:" << e.what() << std::endl; }

	// load and compile the shader
	try { mShader = gl::GlslProg( loadAsset("shader_vert.glsl"), loadAsset("shader_frag.glsl") ); }
	catch( const std::exception &e ) { console() << "Could not load&compile shader:" << e.what() << std::endl; }

	// setup the camera
	CameraPersp cam;
	cam.setEyePoint( Vec3f(150.0f, 100.0f, 150.0f) );
	cam.setCenterOfInterestPoint( Vec3f(50.0f, 0.0f, 50.0f) );
	cam.setNearClip(5.0f);
	cam.setFarClip(1000.0f);
	mMayaCam.setCurrentCam( cam );
}

void SmoothMeshApp::update()
{
	// animate light
	float x = 50.0f + 100.0f * (float) sin( 0.20 * getElapsedSeconds() );
	float y = 50.0f +  45.0f * (float) sin( 0.13 * getElapsedSeconds() );
	float z = 50.0f + 100.0f * (float) cos( 0.20 * getElapsedSeconds() );

	mLightPosition = Vec3f(x, y, z);
}

void SmoothMeshApp::draw()
{
	gl::clear( Color::black() ); 

	gl::pushMatrices();
	gl::setMatrices( mMayaCam.getCamera() );

	// remember what OpenGL effects are enabled by default,
	// so we can restore that situation at the end of the draw()
	glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT );	

	// enable the depth buffer
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// draw origin and axes
	gl::drawCoordinateFrame(15.0f, 2.5f, 1.0f);

	// draw light position
	gl::color( Color(1.0f, 0.5f, 0.0f) );
	gl::drawSphere( mLightPosition, 0.5f );
	gl::drawLine( mLightPosition, Vec3f(mLightPosition.x, 0.0f, mLightPosition.z) );
	gl::drawLine( mLightPosition, Vec3f(50.0f, 0.0f, 50.0f) );

	// the annoying thing with lights is that they will automatically convert the
	// specified coordinates to world coordinates, which depend on the 
	// current camera matrices. So you have to keep telling where the lights are
	// every time the camera moves, otherwise the light will move along
	// with the camera, which causes undesired behavior.
	// To avoid this, and quite a few other annoying things with lights,
	// I usually redefine them every frame, using a function to keep things tidy.
	enableLights();

	// render the shadow map and bind it to texture unit 0,
	// so the shader can access it
	renderShadowMap();
	mDepthFbo.bindDepthTexture(0);

	// enable texturing and bind texture to texture unit 1
	gl::enable( GL_TEXTURE_2D );
	if(mTexture) mTexture.bind(1);

	// bind the shader and set the uniform variables
	if(mShader) {
		mShader.bind();
		mShader.uniform("tex0", 0);
		mShader.uniform("tex1", 1);
		mShader.uniform("flat", mDrawFlatShaded);
		mShader.uniform("shadowMatrix", mShadowMatrix);
	}
	
	// draw the mesh
	if(mDrawWireframe) 
		gl::enableWireframe();

	gl::color( Color::white() );
	gl::draw( mTriMesh );

	if(mDrawWireframe)
		gl::disableWireframe();

	// unbind the shader
	if(mShader) mShader.unbind();

	// unbind the texture
	if(mTexture) mTexture.unbind();

	//
	mDepthFbo.unbindTexture();

	// no need to call 'disableLights()' or 'gl::disableDepthRead/Write()' here, 
	// glPopAttrib() will do it for us

	// restore OpenGL state
	glPopAttrib();

	gl::popMatrices();

	if(mDrawShadowMap) {
		Surface32f shadowMapSurface( mDepthFbo.getDepthTexture() );
		ip::hdrNormalize( &shadowMapSurface );
		gl::color( Color::white() );
		gl::draw( gl::Texture( shadowMapSurface ), Rectf( 0, 128, 128, 0 ) );
	}
}

void SmoothMeshApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void SmoothMeshApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void SmoothMeshApp::keyDown( KeyEvent event )
{
	switch( event.getCode() )
	{
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_m:
		mDrawShadowMap = !mDrawShadowMap;
		break;
	case KeyEvent::KEY_s:
		mDrawFlatShaded = !mDrawFlatShaded;
		break;
	case KeyEvent::KEY_w:
		mDrawWireframe = !mDrawWireframe;
		break;
	case KeyEvent::KEY_1:
		setupMesh(1);
		break;
	case KeyEvent::KEY_2:
		setupMesh(2);
		break;
	case KeyEvent::KEY_3:
		setupMesh(3);
		break;
	}
}

void SmoothMeshApp::resize( )
{
	CameraPersp cam = mMayaCam.getCamera();
	//cam.setAspectRatio( getAspectRatio() );

	mMayaCam.setCurrentCam( cam );
}

void SmoothMeshApp::enableLights()
{
	// setup light 0
	gl::Light light(gl::Light::POINT, 0);

	light.lookAt( mLightPosition, Vec3f( 50.0f, 0.0f, 50.0f ) );
	light.setAmbient( Color( 0.02f, 0.02f, 0.02f ) );
	light.setDiffuse( Color( 1.0f, 1.0f, 1.0f ) );
	light.setSpecular( Color( 1.0f, 1.0f, 1.0f ) );
	light.setShadowParams( 75.0f, 10.0f, 500.0f );
	light.enable();

	// enable lighting
	gl::enable( GL_LIGHTING );

	// because I chose to redefine the light every frame,
	// the easiest way to access the light's shadow settings
	// is to store them in a few member variables
	mShadowMatrix = light.getShadowTransformationMatrix( mMayaCam.getCamera() );
	mShadowCamera = light.getShadowCamera();
}

void SmoothMeshApp::disableLights()
{
	gl::disable( GL_LIGHTING );
}

void SmoothMeshApp::setupShadowMap()
{
	static const int size = 2048;

	// create a frame buffer object (FBO) containing only a depth buffer
	mDepthFbo = gl::Fbo( size, size, false, false, true );

	// set it up for shadow mapping
	mDepthFbo.bindDepthTexture();
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );	
}

void SmoothMeshApp::renderShadowMap()
{
	// store the current OpenGL state, 
	// so we can restore it when done
	glPushAttrib( GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT );

	// bind the shadow map FBO
	mDepthFbo.bindFramebuffer();

	// set the viewport to the correct dimensions and clear the FBO
	glViewport( 0, 0, mDepthFbo.getWidth(), mDepthFbo.getHeight() );
	glClear( GL_DEPTH_BUFFER_BIT );

	// to reduce artefacts, offset the polygons a bit
	glPolygonOffset( 3.0f, 3.0f );
	glEnable( GL_POLYGON_OFFSET_FILL );

	// render the mesh
	gl::enableDepthWrite();

	gl::pushMatrices();
		gl::setMatrices( mShadowCamera );
		gl::draw( mTriMesh );
	gl::popMatrices();

	// unbind the FBO and restore the OpenGL state
	mDepthFbo.unbindFramebuffer();

	glPopAttrib();
}

void SmoothMeshApp::setupMesh(uint32_t model)
{
	// perlin noise generator (see below)
	Perlin	perlin( 8, clock() & 65535 );

	// clear the mesh
	mTriMesh.clear();

	// create the vertices and texture coords
	size_t width = 100;
	size_t depth = 100;

	for(size_t z=0;z<=depth;++z) {
		for(size_t x=0;x<=width;++x) {
			float y = 0.0f;

			switch( model ) {
			case 1:
				//	1. random bumps
				y = 2.5f * Rand::randFloat();
				break;
			case 2:
				//	2. smooth bumps (egg container)
				y = 5.0f * sinf( (float) M_PI * 0.05f * x ) * cosf( (float) M_PI * 0.05f * z );
				break;
			case 3:
				//	3. perlin noise
				y = 20.0f * perlin.fBm( Vec3f( static_cast<float>(x), static_cast<float>(z), 0.0f ) * 0.02f ); 
				break;
			}

			mTriMesh.appendVertex( Vec3f( static_cast<float>(x), y, static_cast<float>(z) ) );
			mTriMesh.appendTexCoord( Vec2f( static_cast<float>(x) / width, static_cast<float>(z) / depth ) );
		}
	}

	// next, create the index buffer
	std::vector<uint32_t>	indices;

	for(size_t z=0;z<depth;++z) {
		size_t base = z * (width + 1);

		for(size_t x=0;x<width;++x) {
			indices.push_back( base + x );	
			indices.push_back( base + x + width + 1 );	
			indices.push_back( base + x + 1 );
			
			indices.push_back( base + x + 1 );		
			indices.push_back( base + x + width + 1 );
			indices.push_back( base + x + width + 2 );
		}
	}

	mTriMesh.appendIndices( &indices.front(), indices.size() );

	// use this custom function to create the normal buffer
	generateNormals(mTriMesh);
}

void SmoothMeshApp::generateNormals(TriMesh		mTriMesh)
{
    mTriMesh.getNormals().clear();
    
    for(size_t i=0;i< mTriMesh.getVertices().size();++i)
        mTriMesh.getNormals().push_back( Vec3f::zero() );
    
    for(size_t i=0;i<mTriMesh.getNumTriangles();++i)
    {
        Vec3f v0 = mTriMesh.getVertices()[ mTriMesh.getIndices()[i * 3] ];
        Vec3f v1 = mTriMesh.getVertices()[ mTriMesh.getIndices()[i * 3 + 1] ];
        Vec3f v2 = mTriMesh.getVertices()[ mTriMesh.getIndices()[i * 3 + 2] ];
        
        Vec3f e0 = (v2 - v0).normalized();
        Vec3f e1 = (v2 - v1).normalized();
        Vec3f n = e0.cross(e1).normalized();
        
        mTriMesh.getNormals()[ mTriMesh.getIndices()[i * 3] ] += n;
        mTriMesh.getNormals()[ mTriMesh.getIndices()[i * 3 + 1] ] += n;
        mTriMesh.getNormals()[ mTriMesh.getIndices()[i * 3 + 2] ] += n;
    }
    
    for(size_t i=0;i<mTriMesh.getNormals().size();++i)
        mTriMesh.getNormals()[i].normalize();
}


CINDER_APP_BASIC( SmoothMeshApp, RendererGl )
