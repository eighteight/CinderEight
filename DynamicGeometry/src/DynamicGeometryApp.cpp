#include "cinder/Camera.h"
//#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/CameraUi.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "Resources.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include "cinder/Timeline.h"
#include "cinder/Rand.h"

#include "cinder/gl/Fbo.h"


#include "cinderSyphon.h"

#define INPUT_DEVICE "Scarlett 2i2 USB"
#define TRANSITION_TIME 20.0

using namespace ci;
using namespace ci::app;
using namespace std;
    int                 mCurrntStep;

class DynamicGeometryApp : public App {
  public:
	enum Primitive { PLANE };
	enum Quality { LOW, DEFAULT, HIGH };
	enum ViewMode { SHADED, WIREFRAME };
	enum TexturingMode { NONE, PROCEDURAL, SAMPLER };

	void prepareSettings( Settings *settings );
	void setup() override;
	void resize() override;
	void update() override;
	void draw() override;

	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;

  private:
	void createGrid();
	void createPhongShader();
	void createWireframeShader();
	void createGeometry();
    void updateFlattness();
	void loadGeomSource( const geom::Source &source );
	void createParams();
    void startTweening();
    void startItunes();
    
    void createFbo();

	Primitive			mPrimitiveSelected;
	Primitive			mPrimitiveCurrent;
	Quality				mQualitySelected;
	Quality				mQualityCurrent;
	ViewMode			mViewMode;

	int					mSubdivision;
	int					mTexturingMode;

	bool				mShowColors;
	bool				mShowNormals;
	bool				mShowGrid;
	bool				mEnableFaceFulling;

	CameraPersp			mCamera;
	CameraUi			mMayaCam;
	bool				mRecenterCamera;
	vec3				mCameraCOI;
	double				mLastMouseDownTime;

	gl::VertBatchRef	mGrid;

	gl::BatchRef		mPrimitive;
	gl::BatchRef		mPrimitiveWireframe;
	gl::BatchRef		mPrimitiveNormalLines;

	gl::GlslProgRef		mPhongShader;
	gl::GlslProgRef		mWireframeShader;
    gl::GlslProgRef     mShader;

	gl::TextureRef		mTexture;
    AxisAlignedBox    mBbox;
    
    float               mFrameRate;
    
	Anim<float>		mMix;
    Anim<float>     mZCam;
    vector<float>   mCamDestinations;
	vector<vec2>	mDestinations;
	
#if ! defined( CINDER_GL_ES )
	params::InterfaceGlRef	mParams;
#endif
    
    
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
    
    // width and height of our mesh
    static const int kWidth = 256;
    static const int kHeight = 256;
    
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    gl::TextureRef			mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;
    uint32_t			mOffset;
    bool                mFlat;
    double              mStartSeconds;

    syphonClient       mClientSyphon;
    
    gl::FboRef mFbo;
};

void DynamicGeometryApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize(1024, 768);
	settings->setHighDensityDisplayEnabled();
	settings->setMultiTouchEnabled( false );
}

void DynamicGeometryApp::createFbo()
{
    // determine the size of the frame buffer
    int w = 1280;//getWindowWidth();
    int h = 720;//getWindowHeight();
    
    if( mFbo && mFbo->getSize() == ivec2( w, h ) )
        return;
    
    // create the FBO
//    gl::Texture2d::Format tfmt;
//    tfmt.setWrap( GL_REPEAT, GL_CLAMP_TO_BORDER );
    gl::Texture::Format tfmt;
    tfmt.setAutoInternalFormat();
    tfmt.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
    
    gl::Fbo::Format fmt;
    fmt.setColorTextureFormat( tfmt );
    
    mFbo = gl::Fbo::create( w, h, fmt );
    
    
    ///
;
    
    
    
    
}

void DynamicGeometryApp::setup()
{
    mClientSyphon.setup();
    
    // in order for this to work, you must run simple server which is a syphon test application
    // feel free to change the app and server name for your specific case
    mClientSyphon.set("MulchCam", "MulchCam");
    
    mClientSyphon.bind();
    
    
	// Initialize variables.
    mStartSeconds = 0.0;
    mCurrntStep = 0;
    mOffset = 0;
    mFrameRate = 0.f;
	mPrimitiveSelected = mPrimitiveCurrent = PLANE;
	mQualitySelected = mQualityCurrent = HIGH;
	mTexturingMode = PROCEDURAL;
	mViewMode = SHADED;
	mLastMouseDownTime = 0;
	mShowColors = false;
	mShowNormals = false;
	mShowGrid = false;
	mEnableFaceFulling = false;

	mSubdivision = 1;
	// Load the textures.
	gl::Texture::Format fmt;
	fmt.setAutoInternalFormat();
	fmt.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	mTexture = gl::Texture::create( loadImage( loadResource( RES_LANDSCAPE_IMAGE_BW)  ), fmt );

	// Setup the camera.
    mZCam = 100.0f;
	mCamera.setEyePoint( normalize( vec3( 3, 3, 6 ) ) * 100.0f );
	mCamera.lookAt( mCameraCOI );

	// Load and compile the shaders.
	createPhongShader();
	createWireframeShader();

	// Create the meshes.
	createGrid();
	createGeometry();

	// Enable the depth buffer.
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Create a parameter window, so we can toggle stuff.
	createParams();

    auto ctx = audio::Context::master();
    std::cout << "Devices available: " << endl;
    for( const auto &dev : audio::Device::getInputDevices() ) {
        std::cout<<dev->getName() <<endl;
    }
    
    std::vector<audio::DeviceRef> devices = audio::Device::getInputDevices();
    const auto dev = audio::Device::findDeviceByName(INPUT_DEVICE);
    if (!dev){
        cout<<"Could not find " << INPUT_DEVICE << endl;
        mInputDeviceNode = ctx->createInputDeviceNode();
        cout<<"Using default input"<<endl;
    } else {
        mInputDeviceNode = ctx->createInputDeviceNode(dev);
    }
    
    cout<< "Using " << mInputDeviceNode->getDevice() -> getName() << endl;
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    mChannelLeft = Channel32f(kBands, kHistory);
    mChannelRight = Channel32f(kBands, kHistory);
    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );

    mTextureFormat.setWrapS( GL_CLAMP_TO_BORDER );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );
    
    mFlat = false;

	// start from current position
    mMix = 0.f;
	mDestinations.push_back( vec2(mMix, 14.0f) );  //plane
    mDestinations.push_back( vec2(0.0f, 48.5f) );  //plane
    mDestinations.push_back( vec2(1.0f, 180.0f) ); //cyl
    mDestinations.push_back( vec2(0.0f, 240.0f) ); //sphere
    mDestinations.push_back( vec2(0.5f, 300.0f) ); //cyl
    
    mCamDestinations.push_back(mZCam);
    mCamDestinations.push_back(3.0f);
    mCamDestinations.push_back(2.7f);
    mCamDestinations.push_back(2.0f);
    mCamDestinations.push_back(100.0f);

    startTweening();
    startItunes();
    
    createFbo();
}

void DynamicGeometryApp::startItunes(){

    std::string cmd = "/usr/local/bin/itunes play";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        cout << "ERROR PLAYING ITUNES"<<endl;
        return;
    };
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    result.erase(remove(result.begin(), result.end(), '\n'), result.end());
    std::cout <<result<<std::endl;

}

void DynamicGeometryApp::update()
{
    
    mFrameRate = getAverageFps();
	// If another primitive or quality was selected, reset the subdivision and recreate the primitive.
	if( mPrimitiveCurrent != mPrimitiveSelected || mQualitySelected != mQualityCurrent ) {
		mSubdivision = 1;
		mPrimitiveCurrent = mPrimitiveSelected;
		mQualityCurrent = mQualitySelected;
		createGeometry();
	}

	// After creating a new primitive, gradually move the camera to get a good view.
	if( false && mRecenterCamera ) {
		float distance = glm::distance( mCamera.getEyePoint(), mCameraCOI );
		mCamera.setEyePoint( mCameraCOI - lerp( distance, 5.0f, 0.1f ) * mCamera.getViewDirection() );
		//mCamera.lookAt( lerp( mCamera.getCenterOfInterestPoint(), mCameraCOI, 0.25f) );
	}
    mCamera.setEyePoint(normalize (mCamera.getEyePoint()) * mZCam.value() );
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);
    
    // increment texture offset
    mOffset = (mOffset+1) % kHistory;
    
    mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
    mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);

    gl::ScopedFramebuffer spec( mFbo);
    gl::clear();
    {
//        const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFbo->getSize() );
//        const gl::ScopedMatrices scopedMatrices;
//        //gl::translate( 0, -0.5 * mFbo->getHeight());
//        mClientSyphon.draw(vec2(0), mFbo->getSize());
        
        gl::ScopedMatrices scopedMatrices;
        gl::setMatricesWindow( mFbo->getSize() );
        gl::ScopedViewport view(vec2(0), mFbo->getSize());
        //gl::translate( 0, -0.5 * mSpectrumFbo->getHeight() );
        mClientSyphon.draw(vec2(0), mFbo->getSize());


    }
}

void nextStep(){
    mCurrntStep ++;
}

void DynamicGeometryApp::startTweening()
{
	//timeline().apply( &mMix, mDestinations[0][0], 0.5f, EaseInOutQuad() );
	for( int i=0; i<mDestinations.size(); i++ ){
		timeline().appendTo( &mMix, mDestinations[i][0], mDestinations[i][1], EaseInOutQuad() ).finishFn(nextStep);
        timeline().appendTo( &mZCam, mCamDestinations[i], mDestinations[i][1], EaseInOutQuad() );
	}
}

void DynamicGeometryApp::updateFlattness(){
    mStartSeconds = getElapsedSeconds();
}

void DynamicGeometryApp::draw()
{
	// Prepare for drawing.
	gl::clear( Color::black() );
//    gl::draw(mFbo->getColorTexture(), Rectf(getWindowBounds()));
//    return;
    gl::enableAlphaBlending();
	gl::setMatrices( mCamera );
	
	// Draw the grid.
	if( mShowGrid && mGrid ) {
		gl::ScopedGlslProg scopedGlslProg( gl::context()->getStockShader( gl::ShaderDef().color() ) );
		// draw the coordinate frame with length 2.
		gl::drawCoordinateFrame( 2 );
		mGrid->draw();
        
       // gl::drawVector(mBbox.getCenter(), mCamera.getCenterOfInterestPoint());
	}

	if( mPrimitive ) {
        gl::setDefaultShaderVars();
		//gl::ScopedTextureBind scopedTextureBind( mTexture );
        
        gl::ScopedTextureBind scopedTextureBind( mFbo->getColorTexture() );
        
        //gl::ScopedGlslProg sh(mShader);
		mPhongShader->uniform( "uTexturingMode", mTexturingMode );

		// Rotate it slowly around the y-axis.
		gl::pushModelView();
		gl::rotate( float( getElapsedSeconds() / 10 ), 0.0f, 1.0f, 0.0f );
		gl::rotate( float( getElapsedSeconds() / 20), 1.0f, 0.0f, 0.0f );
        gl::rotate( float( getElapsedSeconds() / 30), 0.0f, 0.0f, 1.0f );
		// Draw the normals.
		if( mShowNormals && mPrimitiveNormalLines ) {
			gl::ScopedColor colorScope( Color( 1, 1, 0 ) );
			mPrimitiveNormalLines->draw();
		}

		// Draw the primitive.
		gl::ScopedColor colorScope( Color( 0.7f, 0.5f, 0.3f ) );

		// (If transparent, render the back side first).
		if( mViewMode == WIREFRAME ) {
			gl::enableAlphaBlending();

			gl::enable( GL_CULL_FACE );
			gl::cullFace( GL_FRONT );

			mWireframeShader->uniform( "uBrightness", 0.5f );
			mPrimitiveWireframe->draw();
		}

		// (Now render the front side.)
		if( mViewMode == WIREFRAME ) {
			gl::cullFace( GL_BACK );

			mWireframeShader->uniform( "uBrightness", 1.0f );
			mPrimitiveWireframe->draw();
			
			gl::disable( GL_CULL_FACE );

			gl::disableAlphaBlending();
		}
		else {
            float off = (mOffset / float(kHistory) - 0.5) * 2.0f;
            mShader->uniform("uTexOffset", off);
            mShader->uniform("shape", mCurrntStep);

            mShader->uniform("uMix", mMix);
            mShader->uniform("resolution", 0.25f*(float)kWidth);
            mShader->uniform("uTex0",0);
            mShader->uniform("uLeftTex", 1);
            mShader->uniform("uRightTex", 2);

            gl::ScopedTextureBind texLeft( mTextureLeft, 1 );
            gl::ScopedTextureBind texRight( mTextureRight, 2 );

            gl::ScopedBlendAdditive blend;
			mPrimitive->draw();

        }
		// Done.
		gl::popModelView();
	}

	// Render the parameter window.
#if ! defined( CINDER_GL_ES )
	if( mParams )
		mParams->draw();
#endif
}

void DynamicGeometryApp::mouseDown( MouseEvent event )
{
	mRecenterCamera = false;

	mMayaCam.setCamera( &mCamera );
	mMayaCam.mouseDown( event.getPos() );

	if( getElapsedSeconds() - mLastMouseDownTime < 0.2f ) {
		mPrimitiveSelected = static_cast<Primitive>( static_cast<int>(mPrimitiveSelected) + 1 );
		createGeometry();
	}

	mLastMouseDownTime = getElapsedSeconds();
}

void DynamicGeometryApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	mCamera = mMayaCam.getCamera();
}

void DynamicGeometryApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	
	if(mWireframeShader)
		mWireframeShader->uniform( "uViewportSize", vec2( getWindowSize() ) );
}

void DynamicGeometryApp::keyDown( KeyEvent event )
{
	switch( event.getCode() ) {
		case KeyEvent::KEY_SPACE:
			mPrimitiveSelected = static_cast<Primitive>( static_cast<int>(mPrimitiveSelected) + 1 );
			createGeometry();
			break;
		case KeyEvent::KEY_c:
			mShowColors = ! mShowColors;
			createGeometry();
			break;
		case KeyEvent::KEY_n:
			mShowNormals = ! mShowNormals;
			break;
		case KeyEvent::KEY_g:
			mShowGrid = ! mShowGrid;
			break;
		case KeyEvent::KEY_q:
			mQualitySelected = Quality( (int)( mQualitySelected + 1 ) % 3 );
			break;
		case KeyEvent::KEY_w:
			if(mViewMode == WIREFRAME)
				mViewMode = SHADED;
			else
				mViewMode = WIREFRAME;
			break;
		case KeyEvent::KEY_f:
			setFullScreen(!isFullScreen());
			break;
		case KeyEvent::KEY_RETURN:
			CI_LOG_V( "reload" );
			createPhongShader();
			createGeometry();
			break;
        case KeyEvent::KEY_b:
            getWindow()->setBorderless(!getWindow()->isBorderless());
            break;
	}
}

void DynamicGeometryApp::createParams()
{
#if ! defined( CINDER_GL_ES )
	vector<string> qualities = { "Low", "Default", "High" };
	vector<string> viewModes = { "Shaded", "Wireframe" };
	vector<string> texturingModes = { "None", "Procedural", "Sampler" };

	mParams = params::InterfaceGl::create( getWindow(), "Dynamic Geometry", toPixels( ivec2( 300, 300 ) ) );
	mParams->setOptions( "", "valueswidth=160 refresh=0.1" );
    
    mParams->addParam("FPS", &mFrameRate, true);

	mParams->addParam( "Quality", qualities, (int*) &mQualitySelected );
	mParams->addParam( "Viewing Mode", viewModes, (int*) &mViewMode );
	mParams->addParam( "Texturing Mode", texturingModes, (int*) &mTexturingMode );

	mParams->addSeparator();

	mParams->addParam( "Subdivision", &mSubdivision ).min( 1 ).max( 50 ).updateFn( [this] { createGeometry(); } );

	mParams->addSeparator();

	mParams->addParam( "Show Grid", &mShowGrid );
	mParams->addParam( "Show Normals", &mShowNormals );
	mParams->addParam( "Show Colors", &mShowColors ).updateFn( [this] { createGeometry(); } );
	mParams->addParam( "Face Culling", &mEnableFaceFulling ).updateFn( [this] { gl::enableFaceCulling( mEnableFaceFulling ); } );
    
    mParams->addSeparator();
    
    mParams->addParam( "flat", &mFlat).updateFn( [this] { updateFlattness(); } );
    
#endif
}

void DynamicGeometryApp::createGrid()
{
	mGrid = gl::VertBatch::create( GL_LINES );
	mGrid->begin( GL_LINES );
	for( int i = -10; i <= 10; ++i ) {
		if( i == 0 )
			continue;

		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		
		mGrid->vertex( float(i), 0.0f, -10.0f );
		mGrid->vertex( float(i), 0.0f, +10.0f );
		mGrid->vertex( -10.0f, 0.0f, float(i) );
		mGrid->vertex( +10.0f, 0.0f, float(i) );
	}
	mGrid->end();
}

void DynamicGeometryApp::createGeometry()
{
	geom::SourceRef primitive;


    mPrimitiveSelected = PLANE;

    ivec2 numSegments;
    switch( mQualityCurrent ) {
        case DEFAULT:	numSegments = ivec2( 10, 10 ); break;
        case LOW:		numSegments = ivec2( 2, 2 ); break;
        case HIGH:		numSegments = ivec2( 100, 100 ); break;
    }

    auto plane = geom::Plane().subdivisions( numSegments );

//    plane.normal( vec3( 0, 0, 1 ) ); // change the normal angle of the plane
//    plane.axes( vec3( 0.70710678118, -0.70710678118, 0 ), vec3( 0.70710678118, 0.70710678118, 0 ) ); // dictate plane u/v axes directly
//    plane.subdivisions( ivec2( 3, 10 ) ).size( vec2( 0.5f, 2.0f ) ).origin( vec3( 0, 1.0f, 0 ) ).normal( vec3( 0, 0, 1 ) ); // change the size and origin so that it is tall and thin, above the y axis.

    loadGeomSource( geom::Plane( plane ) );
}

void DynamicGeometryApp::loadGeomSource( const geom::Source &source )
{
	// The purpose of the TriMesh is to capture a bounding box; without that need we could just instantiate the Batch directly using primitive
	TriMesh::Format fmt = TriMesh::Format().positions().normals().texCoords();
	if( mShowColors && source.getAvailableAttribs().count( geom::COLOR ) > 0 )
		fmt.colors();

	TriMesh mesh( source, fmt );
	mBbox = mesh.calcBoundingBox();
	mCameraCOI = mesh.calcBoundingBox().getCenter();
	mRecenterCamera = true;

	if( mSubdivision > 1 )
		mesh.subdivide( mSubdivision );

	if( mPhongShader )
		mPrimitive = gl::Batch::create( mesh, mShader );

	if( mWireframeShader )
		mPrimitiveWireframe = gl::Batch::create( mesh, mWireframeShader );

	vec3 size = mBbox.getMax() - mBbox.getMin();
	float scale = std::max( std::max( size.x, size.y ), size.z ) / 25.0f;
	mPrimitiveNormalLines = gl::Batch::create( mesh >> geom::VertexNormalLines(scale ), gl::getStockShader( gl::ShaderDef().color() ) );

	getWindow()->setTitle( "Geometry - " + to_string( mesh.getNumVertices() ) + " vertices" );
}

void DynamicGeometryApp::createPhongShader()
{
	try {
#if defined( CINDER_GL_ES )
		mPhongShader = gl::GlslProg::create( loadAsset( "phong_es2.vert" ), loadAsset( "phong_es2.frag" ) );
#else
		mPhongShader = gl::GlslProg::create( loadAsset( "phong.vert" ), loadAsset( "phong.frag" ) );
        mShader = gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG1 ) );
#endif
	}
	catch( Exception &exc ) {
		CI_LOG_E( "error loading phong shader: " << exc.what() );
	}
}

void DynamicGeometryApp::createWireframeShader()
{
#if ! defined( CINDER_GL_ES )
	try {
		auto format = gl::GlslProg::Format()
			.vertex( loadAsset( "wireframe.vert" ) )
			.geometry( loadAsset( "wireframe.geom" ) )
			.fragment( loadAsset( "wireframe.frag" ) );

		mWireframeShader = gl::GlslProg::create( format );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "error loading wireframe shader: " << exc.what() );
	}
#endif // ! defined( CINDER_GL_ES )
}

CINDER_APP( DynamicGeometryApp, RendererGl )
