// Includes
#include "cinder/app/AppBasic.h"
#include "cinder/Arcball.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ImageIo.h"
#include "cinder/params/Params.h"
#include "cinder/Perlin.h"
#include "cinder/Utilities.h"
#include "Resources.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/qtime/MovieWriter.h"

// Geometry shader sample application
class GeomShaderApp : public ci::app::AppBasic 
{

public:

	// Cinder callbacks
	void draw();
	void mouseDown(ci::app::MouseEvent event);
	void mouseDrag(ci::app::MouseEvent event);
	void mouseMove(ci::app::MouseEvent event);
	void mouseWheel(ci::app::MouseEvent event);
	void prepareSettings(ci::app::AppBasic::Settings * settings);
	void resize();
	void setup();
	void shutdown();
	void update();

private:

	// Cursor
	ci::Vec2i mCursor;

	// Camera
	ci::Arcball mArcball;
	ci::CameraPersp mCamera;
	ci::Vec3f mEyePoint;
	ci::Vec3f mLookAt;

	// Mesh parameters
	int32_t mMeshLength;
	ci::Vec3f mMeshOffset;
	bool mMeshPerlin;
	ci::Vec3f mMeshRotation;
	float mMeshScale;
	float mMeshWaveAmplitude;
	float mMeshWaveSpeed;
	float mMeshWaveWidth;
	int32_t mMeshWidth;

	// Triangle
	float mTriangleRotationSpeed;
	float mTriangleSize;

	// Shader
	ci::gl::GlslProg mShader;
	
	// VBO
	void initMesh();
	std::vector<uint32_t> mVboIndices;
	ci::gl::VboMesh::Layout mVboLayout;
	std::vector<ci::Vec2f> mVboTexCoords;
	std::vector<ci::Vec3f> mVboVertices;
	ci::gl::VboMesh	mVboMesh;

	// Perlin noise
	ci::Perlin mPerlin;

	// Window
	float mElapsedFrames;
	float mElapsedSeconds;
	float mFrameRate;
	bool mFullScreen;
	bool mFullScreenPrev;
	// bool mShowCursor;
	// bool mShowCursorPrev;

	// Debug
	ci::params::InterfaceGl mParams;
	void screenShot();
	void trace(const std::string & message);
    
    bool isWriting;
    cinder::Surface				mSurface;
    cinder::fs::path savePath;
    
    cinder::qtime::MovieWriterRef	mMovieWriter;

};


// Import namespaces
using namespace ci;
using namespace ci::app;
using namespace std;

// Render
void GeomShaderApp::draw()
{

	// Set up scene
	gl::setMatrices(mCamera);
	gl::clear(ColorAf::black(), true);
	gl::setViewport(getWindowBounds());
	
	// Move into position
	gl::pushModelView();
	gl::translate(mMeshOffset);
	gl::rotate(mMeshRotation);

	// Bind and configure shader
	
	 mShader.bind();
	mShader.uniform("amp", mMeshWaveAmplitude);
	mShader.uniform("phase", mElapsedSeconds);
	mShader.uniform("rotation", mTriangleRotationSpeed);
	mShader.uniform("scale", mMeshScale);
	mShader.uniform("size", mTriangleSize);
	mShader.uniform("speed", mMeshWaveSpeed);
	mShader.uniform("width", mMeshWaveWidth);
	
	// Draw VBO
	gl::color(ColorAf::white());
	gl::draw(mVboMesh);

	// Stop drawing
    gl::popModelView();
	mShader.unbind();

	// Draw the params interface
	mParams.draw();

}

// Create mesh
void GeomShaderApp::initMesh()
{

	// Set VBO data
	float delta = 0.001f;
	float theta = 0.0f;
	for (int32_t x = 0; x < mMeshWidth; x++) 
		for (int32_t y = 0; y < mMeshLength; y++) 
		{	
			mVboIndices.push_back(x * mMeshLength + y);
			mVboTexCoords.push_back(Vec2f((float)x / (float)mMeshWidth, (float)y / (float)mMeshLength));
			Vec3f position((float)x - (float)mMeshWidth * 0.5f, (float)y - (float)mMeshLength * 0.5f, 0.0f);
			if (mMeshPerlin)
				position = mPerlin.dnoise(position.x, position.y, math<float>::sin(theta));
			mVboVertices.push_back(position);
			theta += delta;
		}

	// Create VBO
	if (mVboMesh)
		mVboMesh.reset();
	mVboMesh = gl::VboMesh(mVboIndices.size(), mVboIndices.size(), mVboLayout, GL_POINTS);
	mVboMesh.bufferIndices(mVboIndices);
	mVboMesh.bufferPositions(mVboVertices);
	mVboMesh.bufferTexCoords2d(0, mVboTexCoords);
	mVboMesh.unbindBuffers();

	// Clean up
	mVboIndices.clear();
	mVboTexCoords.clear();
	mVboVertices.clear();

	// Resize to reset view
	resize();

}

// Handles mouse down event
void GeomShaderApp::mouseDown(MouseEvent event)
{

	// Set cursor position
	mCursor = event.getPos();

	// Update arcball
	mArcball.mouseDown(event.getPos());
	if (event.isAltDown())
		mMeshRotation = mArcball.getQuat().v;

}

// Handles mouse drag event
void GeomShaderApp::mouseDrag(MouseEvent event)
{

	// Get velocity
	Vec2i velocity = mCursor - event.getPos();

	// Set cursor position
	mCursor = event.getPos();

	// Update arcball
	mArcball.mouseDrag(event.getPos());
	if (event.isAltDown())
		mMeshRotation = mArcball.getQuat().v * math<float>::abs(mEyePoint.z);
	else
		mMeshOffset += Vec3f((float)velocity.x, (float)velocity.y, 0.0f);

}

// Handles mouse move event
void GeomShaderApp::mouseMove(MouseEvent event)
{

	// Set cursor position
	mCursor = event.getPos();

}

// Handles mouse wheel
void GeomShaderApp::mouseWheel(MouseEvent event)
{

	// Zoom
	mMeshScale += event.getWheelIncrement();

}

// Prepare window settings
void GeomShaderApp::prepareSettings(ci::app::AppBasic::Settings * settings)
{

	// Do settings
	settings->setTitle("GeomShaderApp");
	settings->setWindowSize(1024, 768);
	settings->setFrameRate(60.0f);
	settings->setFullScreen(false);

}

// Handles window resize event
void GeomShaderApp::resize()
{

	// Reset camera
	mEyePoint = Vec3f(0.0f, 0.0f, -500.0f);
	mLookAt = Vec3f::zero();
	mCamera.lookAt(mEyePoint, mLookAt);
	mCamera.setPerspective(60.0f, getWindowAspectRatio(), 0.1f, 15000.0f);
	gl::setMatrices(mCamera);

	// Set arcball
	mArcball.setWindowSize(getWindowSize());
	mArcball.setCenter(getWindowCenter());
	mArcball.setRadius((float)getWindowSize().y * 0.5f);

	// Set up OpenGL
	glEnable(GL_DEPTH_TEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	gl::enableAlphaBlending();

}

// Take screen shot
void GeomShaderApp::screenShot()
{

	// DO IT!
	writeImage(getAppPath().string() + "frame" + toString(getElapsedFrames()) + ".png", copyWindowSurface());

}

// Set up
void GeomShaderApp::setup()
{

	// Find maximum number of output vertices for geometry shader
	int32_t maxGeomOutputVertices;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, & maxGeomOutputVertices);

	// Load shader
	mShader = gl::GlslProg(loadResource(RES_SHADER_DRAW_VERT), loadResource(RES_SHADER_DRAW_FRAG), loadResource(RES_SHADER_DRAW_GEOM), GL_POINTS, GL_TRIANGLE_STRIP, maxGeomOutputVertices);

	// Set VBO layout
	mVboLayout.setStaticIndices();
	mVboLayout.setStaticPositions();
	mVboLayout.setStaticTexCoords2d();

	// Set up camera
	mEyePoint = Vec3f::zero();
	mLookAt = Vec3f::zero();
	
	// Set flags
	mFullScreen = isFullScreen();
	mFullScreenPrev = mFullScreen;
	// mShowCursor = true;
	// mShowCursorPrev = true;

	// Set default mesh dimensions
	mMeshLength = 64;
	mMeshOffset = Vec3f(-70.0f, -140.0f, 0.0f);
	mMeshPerlin = false;
	mMeshRotation = Vec3f(75.0f, 0.0f, 315.0f);
	mMeshScale = 10.0f;
	mMeshWaveAmplitude = (float)M_PI;
	mMeshWaveSpeed = 0.667f;
	mMeshWaveWidth = 0.067f;
	mMeshWidth = 256;
	
	// Set default triangle dimensions
	mTriangleRotationSpeed = 0.06667f;
	mTriangleSize = 5.0f;

	// Set up parameters
	mParams = params::InterfaceGl("Parameters", Vec2i(250, 500));
	mParams.addSeparator("");
	mParams.addText("MESH");
	mParams.addParam("Length", & mMeshLength, "min=1 max=2048 step=1 keyDecr=l keyIncr=L");
	mParams.addParam("Offset", & mMeshOffset);
	mParams.addParam("Perlin", & mMeshPerlin, "key=p");
	mParams.addParam("Rotation", & mMeshRotation);
	mParams.addParam("Scale", & mMeshScale, "min=0.1 max=30000.0 step=0.1 keyDecr=c keyIncr=C");
	mParams.addParam("Wave amplitude", & mMeshWaveAmplitude, "min=0.000 max=30000.000 step=0.001 keyDecr=a keyIncr=A");
	mParams.addParam("Wave speed", & mMeshWaveSpeed, "min=0.000 max=100.000 step=0.001 keyDecr=s keyIncr=S");
	mParams.addParam("Wave width", & mMeshWaveWidth, "min=0.000 max=30000.000 step=0.001 keyDecr=v keyIncr=V");
	mParams.addParam("Width", & mMeshWidth, "min=1 max=2048 step=1 keyDecr=w keyIncr=W");
	//mParams.addButton("Reset", std::bind(& GeomShaderApp::initMesh, this), "key=r");
	mParams.addSeparator("");
	mParams.addText("TRIANGLE");
	mParams.addParam("Rotation speed", & mTriangleRotationSpeed, "min=0.001 max=300.0 step=0.001 keyDecr=t keyIncr=T");
	mParams.addParam("Size", & mTriangleSize, "min=0.00001 max=300.00000 step=0.00001 keyDecr=g keyIncr=G");
	mParams.addSeparator("");
	mParams.addText("Hold ALT to rotate");
	mParams.addSeparator("");
	mParams.addText("APPLICATION");
	mParams.addParam("Frame rate ", & mFrameRate, "", true);
	mParams.addParam("Full screen", & mFullScreen, "key=f");
	//mParams.addButton("Save screen shot", std::bind(& GeomShaderApp::screenShot, this), "key=space");
	// mParams.addParam("Show mouse", & mShowCursor, "key=m");
	//mParams.addButton("Quit", std::bind(& GeomShaderApp::quit, this), "key=esc");
	
	// Create mesh
	initMesh();
    
    savePath = getSaveFilePath();
    cinder::qtime::MovieWriter::Format format;
	if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadAsset("black.jpg") ) ) ) {
		mMovieWriter = qtime::MovieWriter::create( savePath, getWindowWidth(), getWindowHeight(), format );
	}
    
    isWriting = false;

}

// Called on exit
void GeomShaderApp::shutdown()
{

	// Clean up
	if (mShader)
		mShader.reset();
	mVboIndices.clear();
	mVboTexCoords.clear();
	mVboVertices.clear();
	if (mVboMesh)
		mVboMesh.reset();

}

// Convenience method for tracing
void GeomShaderApp::trace(const string & message)
{

	// Write to debug window
	console() << message << "\n";
	// OutputDebugStringA(message.c_str());
	// OutputDebugStringA("\n");

}

// Runs update logic
void GeomShaderApp::update()
{

	// Update frame rate and elapsed time
	mElapsedFrames = (float)getElapsedFrames();
	mElapsedSeconds = (float)getElapsedSeconds();
	mFrameRate = getAverageFps();

	// Toggle cursor
	/*
	 if (mShowCursor != mShowCursorPrev)
	{
		if (mShowCursor)
			while (ShowCursor(TRUE) < 0);
		else
			while (ShowCursor(FALSE) >= 0);
		mShowCursorPrev = mShowCursor;
	}
	*/
	
	// Toggle fullscreen
	if (mFullScreen != mFullScreenPrev)
	{
		setFullScreen(mFullScreen);
		//mFullScreenPrev = mFullScreen;
	}

	// Limit scale
	mMeshScale = math<float>::max(mMeshScale, 0.1f);
    
    if( mMovieWriter && isWriting) {
        mMovieWriter->addFrame( copyWindowSurface() );
    }

}

// Run application
CINDER_APP_BASIC(GeomShaderApp, RendererGl)
