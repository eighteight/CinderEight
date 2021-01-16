#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Log.h"
#include "cinder/CameraUi.h"
#include "cinder/Utilities.h"

#include "CinderOpenCV.h"

#include "cinderSyphon.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static std::string const VideoStreamAddress = "rtsp://10.0.1.1:8554/main";
//static std::string const VideoStreamAddress = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";

static const float kVideoSphereRadius = 32.0f;
static const float kCamDistanceMax = 0.0f;

static const float kTargetSize = 2.0f;
static const float kTargetDistance = 4.0f;
static const float kHorizontalSize = kTargetSize * 4.0f;
static const float kRotationAccel = 1.5f;

constexpr static const float FBO_WIDTH = 1920, FBO_HEIGHT=1080;

class RTSPCaptureApp : public App {
  public:
    static void prepareSettings ( Settings* settings );
	void setup() override;
	void update() override;
	void draw() override;
    void resize() override;
    void mouseWheel( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void rotateLeft();
    void rotateRight();
    void renderToFbo();
	
	gl::TextureRef		mTexture;
    gl::BatchRef        mVideoSphere;
    cv::VideoCapture mVideoCapture;
    cv::Mat mCurrentVideoFrame;
    
    CameraPersp   mCamPersp;
    CameraUi      mCamUi;
    float         mCamDistance;
    Anim<float>         mRotation = 0.0f;
    ivec2                        mLastMousePos;
    
    gl::FboRef          mFbo;

    syphonServer       mServerSyphon, mRawTextureSyphon;
};

void RTSPCaptureApp::prepareSettings( Settings* settings )
{
    settings->setResizable( false );
    settings->setWindowSize( 960, 540 );
    settings->setFrameRate(60.0f);
}

void RTSPCaptureApp::setup()
{
    gl::enableVerticalSync( true );
    try {
        if (!mVideoCapture.open(VideoStreamAddress)) {
            console() << "couldn't open stream" << std::endl;
        }
    }
    catch( ... ) {
        console() << "Failed to initialize capture" << std::endl;
    }
    
    float adjustedViewWidth = ( (kHorizontalSize / kTargetSize) * getWindowAspectRatio() );
    float theta = 2.0f * math<float>::atan2( adjustedViewWidth / 2.0f, kTargetDistance );
    
    mCamPersp.setPerspective( toDegrees( theta ) , getWindowAspectRatio(), 1, 1000 );
    mCamDistance = kCamDistanceMax;
    mCamPersp.lookAt( vec3( 0 ) );
    //mCamPersp.setEyePoint( vec3( 0.0f, 0.0f, mCamDistance ) );
    mCamUi            = CameraUi( &mCamPersp );
    mCamUi.connect( getWindow() );
    getWindow()->setTitle("RTSP Capture: " + VideoStreamAddress + " Vladimir Gusev");
    mVideoSphere = gl::Batch::create (geom::Sphere().subdivisions(64).radius(kVideoSphereRadius), gl::getStockShader( gl::ShaderDef().texture() ));
    
    mFbo = gl::Fbo::create( FBO_WIDTH, FBO_HEIGHT, gl::Fbo::Format().disableDepth().colorTexture() );
    
    mServerSyphon.setName("360");
    mRawTextureSyphon.setName("RAW");
}

void RTSPCaptureApp::resize()
{
    mCamPersp.setAspectRatio( getWindowAspectRatio() );
}

void RTSPCaptureApp::mouseDown( MouseEvent event )
{
    //mCamUi.mouseDown( event );
}

void RTSPCaptureApp::mouseMove( MouseEvent event )
{
    mLastMousePos = event.getPos();
}

void RTSPCaptureApp::mouseDrag( MouseEvent event )
{
//    Rectf r    = Rectf( getWindowWidth() / 2, 0, getWindowWidth(), getWindowHeight() );
//    if ( r.contains( event.getPos() )) {
        //mCamUi.mouseDrag( event );
    //}
}

void RTSPCaptureApp::mouseWheel( MouseEvent event ){
    console() << "mouse" << event.getWheelIncrement()<<std::endl;
    mCamDistance += event.getWheelIncrement();
}

void RTSPCaptureApp::rotateLeft(){
    float target = mRotation() > 0.0f ? mRotation() + kRotationAccel : kRotationAccel;
    timeline().apply(&mRotation, target, 2.0f, EaseOutQuad());
}

void RTSPCaptureApp::rotateRight() {
    float target = mRotation() < 0.0f ? mRotation() - kRotationAccel : -kRotationAccel;
    timeline().apply(&mRotation, target, 2.0f, EaseOutQuad());
}

//void RTSPCaptureApp::mouseDrag( MouseEvent event ){
//    console() << "mouse " << event.getX()<<std::endl;
//    rotateRight();
//}

void RTSPCaptureApp::update()
{
    if (!mVideoCapture.read(mCurrentVideoFrame)) {
            console() << "couldn't read current frame from video" << std::endl;
    }
    
    mTexture = gl::Texture::create(fromOcv(mCurrentVideoFrame));
    
//    mCamPersp.setEyePoint( vec3( 0.0f, 0.0f, mCamDistance ) );
//    mCamPersp.setOrientation(quat(1.0, 0., 0., 0.));
    renderToFbo();
    getWindow()->setTitle("RTSP Capture: " + VideoStreamAddress + " Vladimir Gusev: "  + toString(getFrameRate()) + " fps");
}

void RTSPCaptureApp::draw()
{
    gl::clear();

    if (!mTexture) {
        console() << "couldn't load current frame into texture" << std::endl;
        return;
    }
    
    Rectf bounds( 0, 0, toPixels(getWindowWidth()), toPixels(getWindowHeight()) );
    gl::ScopedMatrices scopedMatrices;
    gl::setMatricesWindow( bounds.getSize() );

    gl::draw( mFbo->getColorTexture(), Rectf( getWindowBounds() ) );
    
    mRawTextureSyphon.publishTexture(mTexture); //publish the screen's output
    mServerSyphon.publishTexture(mFbo->getColorTexture(), false);
}


void RTSPCaptureApp::renderToFbo()
{
    if (!mTexture) {
        console() << "couldn't load current frame into texture" << std::endl;
        return;
    }
    gl::ScopedFramebuffer spec( mFbo);
    gl::clear();
    gl::ScopedViewport viewport(mFbo->getSize());

    gl::pushMatrices();
    gl::enableDepthRead();
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatrices(mCamPersp);
    
    gl::ScopedColor col(Color::white());
    gl::ScopedTextureBind tex0 ( mTexture );

    //gl::rotate(mRotation /** getElapsedSeconds() * 0.1f*/, vec3(0,1,0));
    mVideoSphere->draw();
    gl::disableDepthRead();
    gl::popMatrices();
}


//CINDER_APP( RTSPCaptureApp, RendererGl )
CINDER_APP( RTSPCaptureApp, RendererGl( RendererGl::Options().msaa( 16 ) ), RTSPCaptureApp::prepareSettings )
