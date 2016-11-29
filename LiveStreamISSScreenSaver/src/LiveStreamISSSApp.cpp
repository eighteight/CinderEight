#include "cinder/app/RendererGl.h"
#include "CinderLiveStreamer.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/app/AppNative.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LiveStreamISSApp : public AppNative {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    void shutdown() override;
	
  protected:
    qtime::MovieSurfaceRef	mMovieRef;
    SurfaceRef				mSurfaceRef;
    Font					mFont;
    string                  mUrl;
    shared_ptr<thread>		mThread;
    void                    startStreamFn();
    bool                    isStarted;
    double                  lastChecked;
    double                  checkInterval;
};

void LiveStreamISSApp::setup()
{
    isStarted = false;
    checkInterval = 10.0;
    mThread = shared_ptr<thread>( new thread( bind( &LiveStreamISSApp::startStreamFn, this ) ) );
    mThread->detach();
    mFont = Font( "Verdana", 24.0f );

}

void LiveStreamISSApp::startStreamFn(){

    mUrl = "http://www.ustream.tv/channel/live-iss-stream";//
    //mUrl = "http://www.ustream.tv/channel/iss-hdev-payload";
    
    string mStream = CinderLiveStreamer::getStreamUrl(mUrl);
    if (mStream != "ERROR") {
        mMovieRef = cinder::qtime::MovieSurface::create( Url(mStream) );
        if (mMovieRef->isPlayable()) {
            mMovieRef->play();
            isStarted = true;
        } else {
            isStarted = false;
        }
    }
}

void LiveStreamISSApp::shutdown(){
    if (mThread && mThread->joinable()) mThread->join();
}

void LiveStreamISSApp::update()
{
    if (mMovieRef && mMovieRef->checkNewFrame()) {
        mSurfaceRef = mMovieRef->getSurface();
    }
    float lastCheckedDelta = getElapsedSeconds() - lastChecked;
    if (!isStarted && lastCheckedDelta > checkInterval){
        lastChecked = getElapsedSeconds();
        mThread = shared_ptr<thread>( new thread( bind( &LiveStreamISSApp::startStreamFn, this ) ) );
        mThread->detach();
    }
}

void LiveStreamISSApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    if (mSurfaceRef){
        gl::draw(gl::Texture::create(*mSurfaceRef), Rectf(0,0,getWindowWidth(), getWindowHeight()));
    } else {
        gl::drawString(mUrl, vec2 (getWindowWidth()/2, getWindowHeight()/2), Color::white(), mFont);
    }
}

CINDER_APP_NATIVE( LiveStreamISSApp, RendererGl )
