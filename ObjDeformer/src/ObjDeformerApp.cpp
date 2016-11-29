#include "Resources.h"

#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Arcball.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Sphere.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Batch.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Shader.h"
#include "cinder/params/Params.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/Fbo.h"

#include "HandOpenCVTrackerOp.h"
#include "Flywheel.h"

#define TEXTURE_FORMAT GL_RGBA32F_ARB
#define ATTACHMENT_INDEX GL_COLOR_ATTACHMENT0

using namespace ci;
using namespace ci::app;

class ObjDeformerApp : public AppNative {
  public:
	void	setup() override;
	void	resize() override;
    void    update() override;

	void	mouseDown( MouseEvent event ) override;
	void	mouseDrag( MouseEvent event ) override;
	void	keyDown( KeyEvent event ) override;

	void	loadObjFile( const fs::path &filePath );
	void	frameCurrentObject();
	void	draw() override;
    void    renderSceneToFbo();
    
    void    resetFBOs();
	
	Arcball			mArcball;
	MayaCamUI		mMayaCam;
	TriMeshRef		mMesh;
	gl::BatchRef	mBatch;
	gl::GlslProgRef	mGlsl;
    
    gl::TextureRef		mTexture;
    gl::TextureRef		mTextPinPong;
    
    float mStartAngle;
    
    params::InterfaceGlRef	mParams;
    float mFrameRate;
    bool mShowWire;
    bool mUseTwist;
    bool mUseShaderTwist;
    bool mEnableBlending;
    double mStartShaderTwistTime;
    double mStartTwistTime;
    
    HandOpenCVTracker tracker;
    float mTrackerPosition;
    
    Flywheel flywheel;
    
    //accum staff
    gl::FboRef			mFbo[2];

	static const int	FBO_WIDTH = 512, FBO_HEIGHT = 512;
    
    uint mWriteFBO, mReadFBO;
    float mSensitivity;
};

void ObjDeformerApp::setup()
{	
#if defined( CINDER_GL_ES )
	mGlsl = gl::GlslProg::create( loadAsset( "shader_es2.vert" ), loadAsset( "shader_es2.frag" ) );
#else
	mGlsl = gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );
#endif

	CameraPersp initialCam;
	initialCam.setPerspective( 45.0f, getWindowAspectRatio(), 0.1, 10000 );
	mMayaCam.setCurrentCam( initialCam );

    mStartAngle = 0.0f;
	loadObjFile( getAssetPath( "8lbs.obj" ) );
    frameCurrentObject();
    
    mParams = params::InterfaceGl::create( getWindow(), "Object Deformer", toPixels( ivec2( 160, 160 ) ) );
    mParams->setOptions( "", "valueswidth=80 refresh=0.1" );
    mFrameRate = 0.f;
    mParams->addParam("FPS", &mFrameRate, true);
    mShowWire = false;
    mParams->addParam( "Wire", &mShowWire );
    
    mUseTwist = false;
    mParams->addParam("Twist", &mUseTwist).updateFn( [=] { mStartTwistTime = getElapsedSeconds(); } );
    
    mUseShaderTwist = false;
    mParams->addParam("STwist", &mUseShaderTwist).updateFn( [=] { mStartShaderTwistTime = getElapsedSeconds(); } );
    mStartShaderTwistTime = 0.0;
    
    mEnableBlending = false;
    mParams->addParam("Alpha", &mEnableBlending);
    
    mSensitivity = 0.5f;
    mParams->addParam("Sens", &mSensitivity).min( 0.0f ).max( 1.0f ).step( 0.1f );
    
    
    gl::Fbo::Format format;
    format.attachment(ATTACHMENT_INDEX, gl::Texture2d::create(FBO_WIDTH, FBO_HEIGHT, gl::Texture2d::Format().internalFormat(TEXTURE_FORMAT)));

    mFbo[0] = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);
    mFbo[1] = gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);
    
//    mFbo[0] = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, gl::Fbo::Format().colorTexture( gl::Texture::Format().internalFormat( TEXTURE_FORMAT ) ).disableDepth() );
//    mFbo[1] = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, gl::Fbo::Format().colorTexture( gl::Texture::Format().internalFormat( TEXTURE_FORMAT ) ).disableDepth() );

    tracker.setup();

    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mWriteFBO = 0;
    mReadFBO = 1;
    
    Surface32f empty = Surface32f( FBO_WIDTH, FBO_HEIGHT, true);
    Surface32f::Iter pixelIter = empty.getIter();
    while( pixelIter.line() ) {
        while( pixelIter.pixel() ) {
            empty.setPixel( pixelIter.getPos(), ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );
        }
    }
    
    //texture for ping ponging
    gl::Texture::Format tFormat;
    tFormat.setInternalFormat(TEXTURE_FORMAT);
    mTextPinPong = gl::Texture::create( empty, tFormat);
    mTextPinPong->setWrap( GL_REPEAT, GL_REPEAT );
    mTextPinPong->setMinFilter( GL_NEAREST );
    mTextPinPong->setMagFilter( GL_NEAREST );
    
    resetFBOs();
    //setFrameRate(1.0f);
}

void ObjDeformerApp::resetFBOs(){
    
	mWriteFBO = 0;
	mReadFBO = 1;
	mFbo[0]->bindFramebuffer();
	mFbo[1]->bindFramebuffer();
	
	glDrawBuffer(ATTACHMENT_INDEX);
	gl::setMatricesWindow( mFbo[0]->getSize(), false );
	//gl::setViewport( mFbo[0]->getBounds() );
    gl::ScopedViewport scpVp( ivec2( 0 ), mFbo[0]->getSize() );
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mTextPinPong->bind();
	gl::draw( mTextPinPong, mFbo[0]->getBounds() );
	mTextPinPong->unbind();
	
	mFbo[1]->unbindFramebuffer();
	mFbo[0]->unbindFramebuffer();
	//mTextPinPong.disable();

}


void ObjDeformerApp::update(){
    tracker.update();
    
    if (tracker.isTracking()){
        getWindow()->setTitle("tracking");
        float delta = (mTrackerPosition > tracker.getTargetPosition().x ? 1.0f : -1.0f) * abs(mTrackerPosition- tracker.getTargetPosition().x);
        flywheel.applyPush(-100*delta);
        cout<<"FLY "<<flywheel.getLocation() << endl;
        
        mTrackerPosition = tracker.getTargetPosition().x;
    } else {
        getWindow()->setTitle(" ");
    }

    mFrameRate = getAverageFps();
    
    if (mUseTwist) {
        double timeF = getElapsedSeconds() - mStartTwistTime;
        mStartAngle = mStartAngle + 0.0001 * timeF * timeF;

       mBatch->replaceVboMesh(gl::VboMesh::create( *mMesh >>geom::Twist( ).startAngle( mStartAngle ).endAngle( 0 ) ));
        mMesh->recalculateNormals();
    }

    renderSceneToFbo();
}

void ObjDeformerApp::resize()
{
	mArcball.setWindowSize( getWindowSize() );
	mArcball.setCenter( vec2( getWindowWidth() / 2.0f, getWindowHeight() / 2.0f ) );
	mArcball.setRadius( 150 );
}

void ObjDeformerApp::mouseDown( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDown( event.getPos() );
	else
		mArcball.mouseDown( event.getPos() );
}

void ObjDeformerApp::mouseDrag( MouseEvent event )
{
	if( event.isAltDown() )
		mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	else
		mArcball.mouseDrag( event.getPos() );
}

void ObjDeformerApp::loadObjFile( const fs::path &filePath )
{
    TriMesh::Format meshFmt = TriMesh::Format().positions().normals().texCoords();
	ObjLoader loader( (DataSourceRef)loadFile( filePath ) );
	mMesh = TriMesh::create( loader, meshFmt );
	if( ! loader.getAvailableAttribs().count( geom::NORMAL ) )
		mMesh->recalculateNormals();
    
    gl::Texture::Format fmt;
    fmt.setAutoInternalFormat();
    fmt.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
    mTexture = gl::Texture::create( loadImage( loadResource( RES_LANDSCAPE_IMAGE)  ), fmt );
    
    mat4 stretch = scale( vec3( 1, 3, 1  ) );
    *mMesh = *mMesh >> geom::Transform( scale( vec3(0.02) ) );

    mBatch = gl::Batch::create(*mMesh, mGlsl );
}

void ObjDeformerApp::frameCurrentObject()
{
	Sphere boundingSphere = Sphere::calculateBoundingSphere( mMesh->getPositions<3>(), mMesh->getNumVertices() );
	
	mMayaCam.setCurrentCam( mMayaCam.getCamera().getFrameSphere( boundingSphere, 10 ) );
}

void ObjDeformerApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'o' ) {
		fs::path path = getOpenFilePath();
		if( ! path.empty() ) {
			loadObjFile( path );
		}
	}
	else if( event.getChar() == 'f' ) {
		frameCurrentObject();
	}
    else if( event.getChar() == 'F' ) {
        setFullScreen(!isFullScreen());
    }
}

//// Render the color cube into the FBO
void ObjDeformerApp::renderSceneToFbo()
{
    double currShaderElapsedTime = getElapsedSeconds()-mStartShaderTwistTime;
    float gray = cinder::math<float>::clamp(currShaderElapsedTime * 0.0001);
    
    float frameNumber = getElapsedFrames();
    float oneOverFrameNum = 1. / frameNumber;
    float A = (1.0 - mSensitivity)*(frameNumber-1.)*oneOverFrameNum;
    float B = (1.0 - mSensitivity)*oneOverFrameNum + mSensitivity;

    //bind framebuffer to write to
    gl::ScopedFramebuffer fbScp( mFbo[mWriteFBO] );

    gl::ScopedViewport scpVp( ivec2( 0 ), mFbo[mWriteFBO]->getSize() );

    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextPinPong->getId(), 0);
    
    //bind the texture to READ from
    glEnable(  mFbo[ mReadFBO ]->getId() );
    mFbo[ mReadFBO ]->bindTexture();
    gl::ScopedActiveTexture scopedTextureBind( mTextPinPong->getTarget() );
    gl::ScopedGlslProg shader( mGlsl );
    
    gl::clear( Color::black() );
    //attachment to WRITE to
    glDrawBuffer( ATTACHMENT_INDEX );
    gl::setMatrices( mMayaCam.getCamera() );

    mTextPinPong->bind();
    mGlsl->uniform("time", (float) (0.01 * currShaderElapsedTime));
    mGlsl->uniform("isTwist", mUseShaderTwist);
    mGlsl->uniform("mTextPinPong",0);
    mGlsl->uniform("A", A);
    mGlsl->uniform("B", B);
    if (mEnableBlending) gl::enableAlphaBlending();
    gl::rotate( mArcball.getQuat() );
    if (mShowWire) gl::enableWireframe();
    //mBatch->draw();
    gl::draw( mBatch->getVboMesh() );
    if (mShowWire) gl::disableWireframe();
    if (mEnableBlending) gl::disableAlphaBlending();

    mTextPinPong->unbind();
    //mFbo[mWriteFBO]-unbindFrameBuffer();
    mFbo[ mReadFBO ]->unbindTexture();
    mWriteFBO = ( mWriteFBO + 1 ) % 2;
    mReadFBO   = ( mWriteFBO + 1 ) % 2;
}


void ObjDeformerApp::draw()
{
    
    gl::clear( Color::black() );
    gl::setMatricesWindow( toPixels( getWindowSize() ) );
    gl::viewport( toPixels( getWindowSize() ) );

    gl::ScopedTextureBind texBind( mFbo[mReadFBO]->getTexture( ATTACHMENT_INDEX ) );
    gl::ScopedGlslProg texShader( gl::getStockShader( gl::ShaderDef().color().texture() ) );
    gl::drawSolidRect( getWindowBounds() );

    
    
    // clear the window to gray
//    gl::clear( Color( 0.35f, 0.35f, 0.35f ) );
//    
//    gl::setMatricesWindow( toPixels( getWindowSize() ) );
//
//    gl::draw(mFbo[mWriteFBO]->getColorTexture(), getWindowBounds());
//
    if( mParams )
        mParams->draw();
}


CINDER_APP_NATIVE( ObjDeformerApp, RendererGl )