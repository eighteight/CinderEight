#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCV.h"
#include "Resources.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/Capture.h"
#include "cinder/Utilities.h"
#include "cinderSyphon.h"
#include "MoveCropWarpApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// We'll create a new Cinder Application by deriving from the AppBasic class
class dragToSyphon : public MoveCropWarpApp {
  public:
	void	setup();
	void	draw();
	
private:
	syphonServer mScreenSyphon;
};

void dragToSyphon::setup()
{
	MoveCropWarpApp:: setup();
	mScreenSyphon.setName("Cinder Screen");
}

void dragToSyphon::draw()
{
	Rectf cropRect = Rectf(0.5f * mTexture.getWidth()+currentShift, 0.0f * mTexture.getHeight(), 1.0f * mTexture.getWidth()+currentShift, 1.0f * mTexture.getHeight());
	int h = cropRect.getY2()-cropRect.getY1();
	int w = cropRect.getX2() - cropRect.getX1();
	
	if (mScreenSyphon.bindToDrawFrameOfSize(w,h)){
	 	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		gl::setMatricesWindow(getWindowBounds().getWidth(), getWindowBounds().getHeight());

		gl::pushModelView();
		gl::multModelView( mTransform );

		mTexture.enableAndBind();

		Rectf warpRect = Rectf(0, 0, mMovie.getWidth(), mMovie.getHeight());

		drawTexturedRect( warpRect, cropRect, true );
		gl::popModelView();

		mTexture.unbind();

		gl::color( Color( 1, 1, 1 ) );
		mScreenSyphon.unbindAndPublish();
	}

	drawContours();

}

CINDER_APP_BASIC( dragToSyphon, RendererGl )
