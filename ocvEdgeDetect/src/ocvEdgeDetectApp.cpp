#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCV.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Utilities.h"
#include "Resources.h"
using namespace ci;
using namespace ci::app;
using namespace std;

class ocvEdgeDetectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    void loadMovieFile( const fs::path &path );

    qtime::MovieSurface		mMovie;
    Surface orig;
    ImageSourceRef drawSurface;
    cv::Mat input, gray, edges;
    fs::path saveDir;
    bool isWorking;
    int32_t frameNum;
};

void ocvEdgeDetectApp::loadMovieFile( const fs::path &moviePath )
{
    try {
        // load up the movie, set it to loop, and begin playing
        mMovie = qtime::MovieSurface( moviePath );
        mMovie.setLoop(false);
        mMovie.seekToStart();
    }
    catch( ... ) {
        console() << "Unable to load the movie." << std::endl;
        mMovie.reset();
    }
}


void ocvEdgeDetectApp::setup()
{
    frameNum = 0;
    isWorking = false;
    saveDir = getFolderPath();
    if( saveDir.empty() ) return;
//    qtime::MovieWriter::Format format;
//    if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadAsset("lava.jpg") ) ) ) {
//        mMovieWriter = qtime::MovieWriter::create( path, 1920, 1080, format );
//    }
    
    fs::path moviePath = getOpenFilePath();
    if( ! moviePath.empty() )
        loadMovieFile( moviePath );

}

void ocvEdgeDetectApp::mouseDown( MouseEvent event )
{
}

void ocvEdgeDetectApp::update()
{
    if( mMovie){
        mMovie.stepForward();

        frameNum++;

        orig = mMovie.getSurface();
        input = cv::Mat( toOcv( orig ) );
        cv::cvtColor( input, gray, CV_RGB2GRAY );
        //cv::Canny( gray, edges, 10, 100, 3 );
        cv::Sobel( gray, edges, CV_8U, 0, 1 );
        drawSurface = fromOcv(edges);

        if( ! saveDir.empty() ) {
            writeImage( saveDir.string()+ "/"+toString( frameNum )+".png", drawSurface, ImageTarget::Options(),"png");
           // writeImage( saveDir.string()+ "/original/"+toString( frameNum )+".png", orig, ImageTarget::Options(),"png");
        }
        isWorking = true;

        if (mMovie.isDone()){
           // shutdown();
        }
    }
    
    cout<<frameNum<<" "<<getElapsedFrames()<<" "<<mMovie.getCurrentTime()<< endl;
    
}

void ocvEdgeDetectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );

    if (isWorking){
        Rectf centeredRect = Rectf( mMovie.getBounds() ).getCenteredFit( getWindowBounds(), true );

        gl::draw(orig,centeredRect);
    }
}

CINDER_APP_NATIVE( ocvEdgeDetectApp, RendererGl )
