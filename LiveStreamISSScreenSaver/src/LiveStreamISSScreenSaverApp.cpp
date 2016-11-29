#include "cinder/app/AppScreenSaver.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Color.h"
#include "CinderLiveStreamer.h"
#include "cinder/qtime/QuickTime.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LiveStreamISSScreenSaverApp : public AppScreenSaver {
  public:
	void setup() override;
	void update() override;
	void draw() override;
    void shutdown() ;
	
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

void LiveStreamISSScreenSaverApp::setup()
{
    isStarted = false;
    checkInterval = 10.0;
    mThread = shared_ptr<thread>( new thread( bind( &LiveStreamISSScreenSaverApp::startStreamFn, this ) ) );
    mThread->detach();
    mFont = Font( "Verdana", 24.0f );

}

void LiveStreamISSScreenSaverApp::startStreamFn(){
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

void LiveStreamISSScreenSaverApp::shutdown(){
    if (mThread && mThread->joinable()) mThread->join();
}

void LiveStreamISSScreenSaverApp::update()
{
    if (mMovieRef && mMovieRef->checkNewFrame()) {
        mSurfaceRef = mMovieRef->getSurface();
    }
    float lastCheckedDelta = getElapsedSeconds() - lastChecked;
    if (!isStarted && lastCheckedDelta > checkInterval){
        lastChecked = getElapsedSeconds();
        mThread = shared_ptr<thread>( new thread( bind( &LiveStreamISSScreenSaverApp::startStreamFn, this ) ) );
        mThread->detach();
    }
}

void LiveStreamISSScreenSaverApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    if (mSurfaceRef){
        gl::draw(gl::Texture::create(*mSurfaceRef), Rectf(0,0,getWindowWidth(), getWindowHeight()));
    } else {
        gl::drawString(mUrl, vec2 (getWindowWidth()/2, getWindowHeight()/2), Color::white(), mFont);
    }
}

CINDER_APP_SCREENSAVER( LiveStreamISSScreenSaverApp, RendererGl )
