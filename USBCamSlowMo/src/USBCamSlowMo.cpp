#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define WIDTH 640
#define HEIGHT 480

class USBCamSlowMo : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    void prepareSettings( Settings *settings );
    void    keyDown( KeyEvent event ) override;
    
  private:
	void printDevices();

	CaptureRef			mCapture;
	gl::TextureRef		mTexture;
    
    Surface mFrame;
    
    std::vector<Surface> surfaceVector;
    uint32_t currentFrame;
    int mSkippedFrames;
    
    // mesure cam fps
    Timer                    mTimer;
    uint32_t                mCamFrameCount;
    float                    mCamFps;
    uint32_t                mCamFpsLastSampleFrame;
    double                    mCamFpsLastSampleTime;
    float                        mFrameRate;
    float                       mQueueSize;
    
    params::InterfaceGlRef        mParams;
};

void USBCamSlowMo::keyDown( KeyEvent event ){
    if (event.getChar() == 'r'){
        surfaceVector.clear();
    }
}

void USBCamSlowMo::setup()
{
    currentFrame = 0;
    mSkippedFrames = 1;
    mTimer = Timer(true);
    mCamFrameCount = 0;
    mCamFps = 0;
    mCamFpsLastSampleFrame = 0;
    mCamFpsLastSampleTime = 0;
    
	printDevices();
    
    
    auto devices = Capture::getDevices();
    cinder::Capture::DeviceRef averMedia = nil;
    for( const auto &device : devices ) {
        if (device->getName() == "AVerMedia USB Device #2") {
            cout<< "here" << endl;
            averMedia = device;
        }
        console() << "Device: " << device->getName() << " "<<
        device->getUniqueId()
        << endl;
    }

	try {
		mCapture = Capture::create( 640, 480, averMedia);
		mCapture->start();
	}
	catch( ci::Exception &exc ) {
		CI_LOG_EXCEPTION( "Failed to init capture ", exc );
	}
    
    mParams = params::InterfaceGl::create( "PS3EYE", toPixels( ivec2( 180, 150 ) ) );
    
    mParams->addParam( "Framerate", &mFrameRate, "", true );
    mParams->addParam( "Queue", &mQueueSize, "", true);
    mParams->addSeparator();
    mParams->addParam( "Skip", &mSkippedFrames).min( 1 ).step( 1 );

}

void USBCamSlowMo::update()
{
	if( mCapture && mCapture->checkNewFrame() ) {
            mFrame = *mCapture->getSurface();
            //Surface source = mFrame.clone();
            
            OStreamMemRef os = OStreamMem::create();
            
            DataTargetRef target = DataTargetStream::createRef( os );
            
            writeImage( target, mFrame, ImageTarget::Options(), "jpeg" );
            
            const void *data = os->getBuffer();
            
            size_t dataSize = os->tell();
            
            BufferRef buf = Buffer::create(dataSize );
            memcpy(buf->getData(), data, dataSize);
            surfaceVector.push_back(Surface( loadImage( DataSourceBuffer::create(buf)), SurfaceConstraintsDefault(), false ));
	}
    mQueueSize = surfaceVector.size();
    currentFrame++;
    
    mFrameRate = getFrameRate();
}

void USBCamSlowMo::draw()
{
	gl::clear();

    gl::ScopedModelMatrix modelScope;

    if (!surfaceVector.empty() && ( currentFrame % mSkippedFrames == 0 )){
        mTexture = gl::Texture::create(surfaceVector.front());
        surfaceVector.erase(surfaceVector.begin());
    }
    if( mTexture ) {
        gl::draw( mTexture );
    }


        mParams->draw();
}

void USBCamSlowMo::printDevices()
{
	for( const auto &device : Capture::getDevices() ) {
		console() << "Device: " << device->getName() << " "<<
        device->getUniqueId()
		<< endl;
	}
}

void prepareSettings( USBCamSlowMo::Settings* settings )
{
    settings->setWindowSize( WIDTH, HEIGHT );
    settings->setFrameRate( 60.0f );
}

CINDER_APP( USBCamSlowMo, RendererGl, prepareSettings )
