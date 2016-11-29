#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Light.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ToonApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void keyDown( KeyEvent event );	
	void update();
	void draw();
	void recalculateNormals(TriMesh* mesh);

	TriMesh arcadeCabinet;
	CameraPersp camera;

	gl::GlslProg depthShader;
	gl::GlslProg convolutionShader;
	gl::GlslProg normalShader;
	gl::GlslProg normalEdgeShader;
	gl::GlslProg silhouetteShader;
	gl::GlslProg compositeShader;
	gl::GlslProg passThruShader;
	gl::GlslProg phongShader;

	gl::Fbo normalBuffer;
	gl::Fbo normalEdgeBuffer;
	gl::Fbo depthBuffer;
	gl::Fbo depthEdgeBuffer;
	gl::Fbo silhouetteBuffer;
	gl::Fbo shadedModelBuffer;

	float cabinetRotation;
	bool viewAllPasses;
	bool drawLitCabinet;
};

void ToonApp::setup()
{
	camera.setPerspective(60.0f, getWindowAspectRatio(), 5.0f, 3000.0f);
	camera.lookAt(Vec3f(0.0f, 0.0f, -500.0f), Vec3f::zero(), -1*Vec3f::yAxis());

	ObjLoader loader(loadAsset("cube.obj"));
	loader.load(&arcadeCabinet, true);
	// Obj loader doesn't properly export normals for the trimesh
	recalculateNormals(&arcadeCabinet);

	try {
		depthShader = gl::GlslProg( loadAsset( "depth_vert.glsl" ), loadAsset( "depth_frag.glsl" ) );
		convolutionShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "convolution_frag.glsl" ) );
		normalShader = gl::GlslProg( loadAsset( "normal_vert.glsl" ), loadAsset( "normal_frag.glsl" ) );
		normalEdgeShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "normal_edge_frag.glsl" ) );
		silhouetteShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "render_silhouette_frag.glsl" ) );
		compositeShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "composite_frag.glsl" ) );
		passThruShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "passThru_frag.glsl" ) );
		phongShader = gl::GlslProg( loadAsset( "phong_vert.glsl" ), loadAsset( "phong_frag.glsl" ) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	catch( ... ) {
		console() << "Unable to load shader" << std::endl;
	}

	gl::Fbo::Format format;
	format.enableMipmapping(false);
	format.setCoverageSamples(16);
	format.setSamples(4);

	depthBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	depthEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	normalBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	normalEdgeBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	silhouetteBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);
	shadedModelBuffer = gl::Fbo(getWindowWidth(), getWindowHeight(), format);

	cabinetRotation = 0.0;
	viewAllPasses = false;
	drawLitCabinet = false;
}

void ToonApp::mouseDown( MouseEvent event )
{	
}

void ToonApp::keyDown( KeyEvent event ) {
	if( event.getCode() == KeyEvent::KEY_v ){
        viewAllPasses = !viewAllPasses;
	} else if( event.getCode() == KeyEvent::KEY_c ){
		drawLitCabinet = !drawLitCabinet;
	}
}

void ToonApp::update()
{
	cabinetRotation += 0.25;
}

void ToonApp::draw()
{
		// Render depth info to a texture
		depthBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color::black(), true);

		depthShader.bind();
			gl::pushMatrices();
				gl::setMatrices(camera);
				gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
				gl::scale(100.0f, 100.0f, 100.0f);
				gl::draw(arcadeCabinet);
			gl::popMatrices();
		depthShader.unbind();
	depthBuffer.unbindFramebuffer();

	gl::clear(Color::white(), true);
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Run an edge detection shader against the depth image to get an
	// outline around the object (Note: this will not give you interior
	// edges, we'll get those from the normal rendering below)
	gl::pushModelView();
		depthEdgeBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			convolutionShader.bind();
				convolutionShader.uniform("depthImage", 0);
				convolutionShader.uniform("textureSizeX", (float)depthBuffer.getTexture().getWidth());
				convolutionShader.uniform("textureSizeY", (float)depthBuffer.getTexture().getHeight());

				depthBuffer.getTexture().bind();
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				depthBuffer.getTexture().unbind();
			convolutionShader.unbind();
		depthEdgeBuffer.unbindFramebuffer();
	gl::popModelView();

	// Render model with faces colored using normals to a texture
	normalBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color::black(), true);

		normalShader.bind();
			gl::pushMatrices();
				gl::setMatrices(camera);
				gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
				gl::scale(100.0f, 100.0f, 100.0f);
				gl::draw(arcadeCabinet);
			gl::popMatrices();
		normalShader.unbind();
	normalBuffer.unbindFramebuffer();
    
	gl::clear(Color::white(), true);
	gl::disableDepthRead();
	gl::disableDepthWrite();

	// Now run an edge detection against the normal texture to get edges,
	// including interior ones
	gl::pushModelView();
		normalEdgeBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			normalEdgeShader.bind();
				normalEdgeShader.uniform("normalImage", 0);
				normalEdgeShader.uniform("textureSizeX", (float)depthBuffer.getTexture().getWidth());
				normalEdgeShader.uniform("textureSizeY", (float)depthBuffer.getTexture().getHeight());
				normalEdgeShader.uniform("normalEdgeThreshold", 0.20f);

				normalBuffer.getTexture().bind();
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				normalBuffer.getTexture().unbind();
			normalEdgeShader.unbind();
		normalEdgeBuffer.unbindFramebuffer();
	gl::popModelView();

	// Now combine the depth edge texture data with the normal edge data,
	// using the stronger value from either texture so that we get a solid 
	// outline and solid interior edges
	gl::pushModelView();
		silhouetteBuffer.bindFramebuffer();
			gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
			gl::scale( Vec3f(1, -1, 1) );
			silhouetteShader.bind();
				silhouetteShader.uniform("normalEdgeImage", 0);
				silhouetteShader.uniform("depthEdgeImage", 1);
				silhouetteShader.uniform("textureSizeX", (float)depthEdgeBuffer.getTexture().getWidth());
				silhouetteShader.uniform("textureSizeY", (float)depthEdgeBuffer.getTexture().getHeight());

				normalEdgeBuffer.getTexture().bind();
				depthEdgeBuffer.getTexture().bind(1);
					gl::color(Color::white());
					gl::drawSolidRect( getWindowBounds() );
				depthEdgeBuffer.getTexture().unbind(1);
				normalBuffer.getTexture().unbind();
			silhouetteShader.unbind();
		silhouetteBuffer.unbindFramebuffer();
	gl::popModelView();

	// Now render the model to a texture
	shadedModelBuffer.bindFramebuffer();
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::clear(Color(255.0, 136.0/255.0, 44.0/255.0), true);
		phongShader.bind();

		gl::pushMatrices();
			gl::setMatrices(camera);
			gl::rotate(Vec3f(180.0, cabinetRotation, 0.0));
			gl::scale(100.0f, 100.0f, 100.0f);
			gl::draw(arcadeCabinet);
		gl::popMatrices();
		phongShader.unbind();
	shadedModelBuffer.unbindFramebuffer();

	// Finally, composite the shaded texture with the combined edge
	// textures to get our final, "Toon Shaded" output.
	gl::pushModelView();
		gl::translate( Vec2f(0.0, (float)getWindowHeight()) );
		gl::scale( Vec3f(1, -1, 1) );
		compositeShader.bind();
			compositeShader.uniform("shadedModelImage", 1);
			compositeShader.uniform("silhouetteImage", 0);
			compositeShader.uniform("silhouetteColor", Vec4f(0.0, 0.0, 0.0, 1.0));

			silhouetteBuffer.getTexture().bind();
			shadedModelBuffer.getTexture().bind(1);
				gl::color(Color::white());
				gl::drawSolidRect( getWindowBounds() );
			silhouetteBuffer.getTexture().unbind(1);
			shadedModelBuffer.getTexture().unbind();
		compositeShader.unbind();
	gl::popModelView();

	// Render smaller views along the left edge of the screen with 
	// each shader pass; useful for debugging.
	if(viewAllPasses){
		int windowHeight = getWindowHeight();
		int viewportHeight = windowHeight / 6;		
		int viewportWidth = getWindowWidth() / 6;
		Area viewport = gl::getViewport();
		
		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight, viewportWidth, windowHeight - viewportHeight));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				depthBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				depthBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight - viewportHeight, viewportWidth, windowHeight - (2*viewportHeight)));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				depthEdgeBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				depthEdgeBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight - (2*viewportHeight), viewportWidth, windowHeight - (3*viewportHeight)));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				normalBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				normalBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight - (3*viewportHeight), viewportWidth, windowHeight - (4*viewportHeight)));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				normalEdgeBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				normalEdgeBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight - (4*viewportHeight), viewportWidth, windowHeight - (5*viewportHeight)));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				silhouetteBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				silhouetteBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::pushModelView();
		gl::setViewport(Area(0, windowHeight - (5*viewportHeight), viewportWidth, windowHeight - (6*viewportHeight)));
			gl::translate( Vec2f(0.0, (float)windowHeight) );
			gl::scale( Vec3f(1, -1, 1) );
			passThruShader.bind();
				passThruShader.uniform("texture", 0);

				shadedModelBuffer.getTexture().bind();
					gl::translate(0.0, 0.0, 0.5);
					gl::color(Color::white());
					gl::drawSolidRect( viewport );
				shadedModelBuffer.getTexture().unbind();
			passThruShader.unbind();
		gl::popModelView();

		gl::setViewport(viewport);
	}
}

// Since ObjLoader or Blender doesn't correctly export normals, we need to create a new mesh based on the old mesh and redo the normals
void ToonApp::recalculateNormals( TriMesh *mesh )
{
	// create an empty normal buffer and fill it with zero-length vectors
	std::vector<Vec3f> normals;
	normals.assign( mesh->getVertices().size(), Vec3f::zero() );

	size_t n = mesh->getNumTriangles();
	for( size_t i = 0; i < n; ++i ) {
		// note: repeatedly calling getIndices() may not seem optimal,
		//  but most compilers will automatically optimize this to using const references.
		uint32_t index0 = mesh->getIndices()[i * 3];
		uint32_t index1 = mesh->getIndices()[i * 3 + 1];
		uint32_t index2 = mesh->getIndices()[i * 3 + 2];
		
		// note: repeatedly calling getVertices() may not seem optimal,
		//  but most compilers will automatically optimize this to using const references.
		Vec3f v0 = mesh->getVertices()[ index0 ];
		Vec3f v1 = mesh->getVertices()[ index1 ];
		Vec3f v2 = mesh->getVertices()[ index2 ];

		Vec3f e0 = v1 - v0;
		Vec3f e1 = v2 - v0;
		Vec3f normal = e0.cross(e1).normalized();

		normals[ index0 ] += normal;
		normals[ index1 ] += normal;
		normals[ index2 ] += normal;
	}

	// normalize all normals
	std::for_each( normals.begin(), normals.end(), std::mem_fun_ref(&Vec3f::normalize) );

	// magic trick: use swap() to exchange the TriMesh's normal buffer for our new one
	mesh->getNormals().swap( normals );
}

CINDER_APP_BASIC( ToonApp, RendererGl )
