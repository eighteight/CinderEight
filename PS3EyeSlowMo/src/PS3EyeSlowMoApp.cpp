#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "ps3eye.h"
#include "cinder/params/Params.h"
#include "ConcurrentQueue.h"
#include "CinderVideoStreamServer.h"
#include "cinder/gl/Fbo.h"
#include "cinder/ImageIo.h"
#include "cinder/DataSource.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 640
#define HEIGHT 480

static const int ITUR_BT_601_CY = 1220542;
static const int ITUR_BT_601_CUB = 2116026;
static const int ITUR_BT_601_CUG = -409993;
static const int ITUR_BT_601_CVG = -852492;
static const int ITUR_BT_601_CVR = 1673527;
static const int ITUR_BT_601_SHIFT = 20;

typedef CinderVideoStreamServer<uint8_t> CinderVideoStreamServerUint8;

static void yuv422_to_rgba(const uint8_t *yuv_src, const int stride, uint8_t *dst, const int width, const int height)
{
    const int bIdx = 0;
    const int uIdx = 0;
    const int yIdx = 0;
    
    const int uidx = 1 - yIdx + uIdx * 2;
    const int vidx = (2 + uidx) % 4;
    int j, i;
    
#define _max(a, b) (((a) > (b)) ? (a) : (b))
#define _saturate(v) static_cast<uint8_t>(static_cast<uint32_t>(v) <= 0xff ? v : v > 0 ? 0xff : 0)
    
    for (j = 0; j < height; j++, yuv_src += stride)
    {
        uint8_t* row = dst + (width * 4) * j; // 4 channels
        
        for (i = 0; i < 2 * width; i += 4, row += 8)
        {
            int u = static_cast<int>(yuv_src[i + uidx]) - 128;
            int v = static_cast<int>(yuv_src[i + vidx]) - 128;
            
            int ruv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVR * v;
            int guv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CVG * v + ITUR_BT_601_CUG * u;
            int buv = (1 << (ITUR_BT_601_SHIFT - 1)) + ITUR_BT_601_CUB * u;
            
            int y00 = _max(0, static_cast<int>(yuv_src[i + yIdx]) - 16) * ITUR_BT_601_CY;
            row[2-bIdx] = _saturate((y00 + ruv) >> ITUR_BT_601_SHIFT);
            row[1]      = _saturate((y00 + guv) >> ITUR_BT_601_SHIFT);
            row[bIdx]   = _saturate((y00 + buv) >> ITUR_BT_601_SHIFT);
            row[3]      = (0xff);
            
            int y01 = _max(0, static_cast<int>(yuv_src[i + yIdx + 2]) - 16) * ITUR_BT_601_CY;
            row[6-bIdx] = _saturate((y01 + ruv) >> ITUR_BT_601_SHIFT);
            row[5]      = _saturate((y01 + guv) >> ITUR_BT_601_SHIFT);
            row[4+bIdx] = _saturate((y01 + buv) >> ITUR_BT_601_SHIFT);
            row[7]      = (0xff);
        }
    }
}

class PS3EyeSlowMoApp : public AppNative {
public:
    void setup();
    void mouseDown( MouseEvent event );
    void update();
    void draw();
    void shutdown();
    void setGain();
    void	keyDown( KeyEvent event );
    
    void eyeUpdateThreadFn();
    
    params::InterfaceGlRef		mParams;
    float						mFrameRate;
    float                       mQueueSize;
    bool                        isAutoGain;
    bool                        isAutoWB;
    
    ps3eye::PS3EYECam::PS3EYERef eye;
    
    bool					mShouldQuit;
    std::thread				mThread;
    
    gl::TextureRef mTexture;
    uint8_t *frame_bgra;
    Surface mFrame;
    
    // mesure cam fps
    Timer					mTimer;
    uint32_t				mCamFrameCount;
    float					mCamFps;
    uint32_t				mCamFpsLastSampleFrame;
    double					mCamFpsLastSampleTime;
    
    ph::ConcurrentQueue<Surface*>* surfaceQueue;
    std::vector<Surface> surfaceVector;
    void threadLoop();
    std::string* mClientStatus;
    bool running;
    void prepareSettings( Settings *settings );
    uint32_t currentFrame;
    int mSkippedFrames;
    
    
    //fbo
    gl::FboRef			mFbo, mAccumFbo;
    
    
};

void PS3EyeSlowMoApp::threadLoop()
{
    while (running) {
//        try {
//            boost::shared_ptr<CinderVideoStreamServerUint8> server = boost::shared_ptr<CinderVideoStreamServerUint8>(new CinderVideoStreamServerUint8(3333,surfaceQueue, WIDTH, HEIGHT));
//            server.get()->run();
//        }
//        catch (std::exception& e) {
//            std::cerr << "Exception: " << e.what() << "\n";
//        }
        //boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

void PS3EyeSlowMoApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( WIDTH, HEIGHT );
    settings->setFrameRate( 60.0f );
}

void PS3EyeSlowMoApp::keyDown( KeyEvent event ){
    if (event.getChar() == 'r'){
        surfaceVector.clear();
    }
}

void PS3EyeSlowMoApp::setup()
{
    currentFrame = 0;
    mSkippedFrames = 1;
    using namespace ps3eye;
    
    mShouldQuit = false;
    
    // list out the devices
    std::vector<PS3EYECam::PS3EYERef> devices( PS3EYECam::getDevices() );
    console() << "found " << devices.size() << " cameras" << std::endl;
    
    mTimer = Timer(true);
    mCamFrameCount = 0;
    mCamFps = 0;
    mCamFpsLastSampleFrame = 0;
    mCamFpsLastSampleTime = 0;
    
    if(devices.size())
    {
        eye = devices.at(0);
        bool res = eye->init(640, 480, 60);
        console() << "init eye result " << res << std::endl;
        eye->start();

        frame_bgra = new uint8_t[eye->getWidth()*eye->getHeight()*4];
        mFrame = Surface(frame_bgra, eye->getWidth(), eye->getHeight(), eye->getWidth()*4, SurfaceChannelOrder::BGRA);
        memset(frame_bgra, 0, eye->getWidth()*eye->getHeight()*4);
        
        // create and launch the thread
        mThread = thread( bind( &PS3EyeSlowMoApp::eyeUpdateThreadFn, this ) );
    }
    
    mParams = params::InterfaceGl::create( "PS3EYE", toPixels( ivec2( 180, 150 ) ) );
    
    mParams->addParam( "Framerate", &mFrameRate, "", true );
    mParams->addParam( "Queue", &mQueueSize, "", true);
    mParams->addSeparator();
    mParams->addParam( "Skip", &mSkippedFrames).min( 1 ).step( 1 );
    mParams->addParam( "Auto gain", &isAutoGain );
    mParams->addParam( "Auto WB", &isAutoWB );

    //surfaceQueue = new ph::ConcurrentQueue<Surface*>();

    mAccumFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(),
                                gl::Fbo::Format().colorTexture( gl::Texture::Format().internalFormat( GL_RGB16F ) ).disableDepth() );
    
}

void PS3EyeSlowMoApp:: setGain(){
    
}

void PS3EyeSlowMoApp::eyeUpdateThreadFn()
{
    while( !mShouldQuit )
    {
        bool res = ps3eye::PS3EYECam::updateDevices();
        if(!res) break;
    }
}

void PS3EyeSlowMoApp::shutdown()
{
    mShouldQuit = true;
    mThread.join();
    // You should stop before exiting
    // otherwise the app will keep working
    eye->stop();
    //
    delete[] frame_bgra;
    //delete gui;
    
}

void PS3EyeSlowMoApp::mouseDown( MouseEvent event )
{
}

void PS3EyeSlowMoApp::update()
{
    if(eye)
    {
        bool isNewFrame = eye->isNewFrame();
        if(isNewFrame)
        {
            yuv422_to_rgba(eye->getLastFramePointer(), eye->getRowBytes(), frame_bgra, mFrame.getWidth(), mFrame.getHeight());
            
            //Surface source = mFrame.clone();
            
            OStreamMemRef os = OStreamMem::create();
            
            DataTargetRef target = DataTargetStream::createRef( os );
            
            writeImage( target, mFrame, ImageTarget::Options(), "jpeg" );

            const void *data = os->getBuffer();
            
            size_t dataSize = os->tell();

            Buffer buf( dataSize );
            memcpy(buf.getData(), data, dataSize);
            surfaceVector.push_back(Surface( loadImage( DataSourceBuffer::create(buf)), SurfaceConstraintsDefault(), false ));
        }
        mCamFrameCount += isNewFrame ? 1 : 0;
        double now = mTimer.getSeconds();
        if( now > mCamFpsLastSampleTime + 1 ) {
            uint32_t framesPassed = mCamFrameCount - mCamFpsLastSampleFrame;
            mCamFps = (float)(framesPassed / (now - mCamFpsLastSampleTime));
            
            mCamFpsLastSampleTime = now;
            mCamFpsLastSampleFrame = mCamFrameCount;
        }
        mFrameRate = eye->getFrameRate();
    }
    
    mQueueSize = surfaceVector.size();
    currentFrame++;
}

void PS3EyeSlowMoApp::draw()
{
    gl::clear( Color::black() );
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::enableAlphaBlending();
    
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
//    Surface currentSur;
//    //surfaceQueue->try_pop(currentSur);
//    currentSur = surfaceVector[0];

    if (!surfaceVector.empty() && ( currentFrame % mSkippedFrames == 0 )){
        mTexture = gl::Texture::create(surfaceVector.front());
        surfaceVector.erase(surfaceVector.begin());
    }
    if( mTexture ) {
        gl::draw( mTexture );
    }

    mParams->draw();
}

CINDER_APP_NATIVE( PS3EyeSlowMoApp, RendererGl )

