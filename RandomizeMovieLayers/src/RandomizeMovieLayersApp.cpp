#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include <boost/filesystem.hpp>
#include <iterator>
#include <iostream>
#include <vector>
#include "cinder/qtime/QuickTime.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RandomizeMovieLayersApp : public App {
  public:
	void setup() override;
    void update() override;
	void draw() override;
	
	gl::TextureRef		mTexture;
    qtime::MovieSurfaceRef    mMovie;
    SurfaceRef                mSurface;
    
    std::vector<int32_t> mMovieVector;
    int32_t mMaxFrames = 0, mCurFrame = 0, mFrameShift = 5, mCurrentMovie = 0;
};

void RandomizeMovieLayersApp::setup()
{
    using namespace boost::filesystem;
	try {
        fs::path path = fs::path("/Users/eigh_io/inst"); //getFolderPath();
        std::vector<directory_entry> v;
		if( ! path.empty() ) {
            copy(directory_iterator(path), directory_iterator(), back_inserter(v));
            for ( std::vector<directory_entry>::const_iterator it = v.begin(); it != v.end();  ++ it )
            {
                std::cout<< (*it).path().string()<<endl;
                fs::path pth = (*it).path();
                std::string moviePath = pth.string();
                qtime::MovieSurfaceRef movie = qtime::MovieSurface::create( moviePath );
                cout << "num frames " <<  movie->getNumFrames() << endl;
                mMaxFrames = movie->getNumFrames() > mMaxFrames ? movie->getNumFrames() : mMaxFrames;

                std::string comm = "/usr/local/bin/ffmpeg -i " + moviePath + " -vf fps=30 -vsync 0 " + pth.replace_extension().string() + "_out%d.png";
                std::system(comm.c_str());
                mMovieVector.push_back(movie->getNumFrames());
            }

		}
	}
	catch( Exception &exc ) {
		CI_LOG_EXCEPTION( "failed to load image.", exc );
	}
    cout << " NUM FRAMES " << mMaxFrames << " " << mMovieVector.size() << endl;
    
    //ffmpeg -i in.mp4 -vf select='between(t,2,6)+between(t,15,24)' -vsync 0 out%d.png
}

void RandomizeMovieLayersApp::update()
{
    if (mMovieVector.size()) {
        
        if ( mCurFrame < mMaxFrames) {
        
            mCurrentMovie = mCurFrame % mFrameShift == 0 ? randInt(0, mMovieVector.size()) : mCurrentMovie;

            //qtime::MovieSurfaceRef    movie = mMovieVector[mCurrentMovie];
//            int32_t movFrame = mCurFrame * movie->getNumFrames() / mMaxFrames;
//
//            float seekTime = movFrame == 0 ? 0.0f : movie ->getDuration() / movFrame;
//            cout<< mCurrentMovie << " " << movie->getDuration() << " " << movFrame << " " << mCurFrame << " " << seekTime << endl;
//
            //movie->play();
            //movie -> seekToTime(seekTime);
            //mSurface = movie->getSurface();
            //movie->stop();
            
            if (mSurface) {
                mCurFrame ++;
                fs::path p = getHomeDirectory() / "render1" / ( "random_" + toString( mCurFrame ) + ".png" );
                writeImage( p, *mSurface );
            }
        } else {
            quit();
        }
    }
}

void RandomizeMovieLayersApp::draw()
{
	gl::clear( Color( 0.5f, 0.5f, 0.5f ) );
	gl::enableAlphaBlending();
	
	if( mSurface ) {
		Rectf destRect = Rectf( mSurface->getBounds() ).getCenteredFit( getWindowBounds(), true ).scaledCentered( 0.85f );
        gl::draw( gl::Texture::create( *mSurface ) , destRect );
	}
}

CINDER_APP( RandomizeMovieLayersApp, RendererGl )
