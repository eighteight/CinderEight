/*
 * HandOpenCVTracker.h
 *
 *  Created on: Apr 17, 2011
 *      Author: eight
 */

#include "HandOpenCVTrackerOp.h"

using namespace cinder;
using namespace ci;
using namespace std;

HandOpenCVTracker::HandOpenCVTracker(){
    
}

HandOpenCVTracker::~HandOpenCVTracker(){
    
}
void HandOpenCVTracker::setup(){
	 tracking = false;
	 mThreshold = 143.0f;
	 mBlobMin = 20.0f;
	 mBlobMax = 80.0f;

	 //mParams = params::InterfaceGl( "Kinect Midi", Vec2i( 10, 10 ) );
//	 mParams = PersistentParams( "Kinect Track", Vec2i( 200, 100 ) );
//	 mParams.addPersistentParam( "Threshold", &mThreshold, (float) 120.0, "min=0.0 max=255.0 step=1.0 keyIncr=s keyDecr=w" );
//	 mParams.addPersistentParam( "Blob Min R", &mBlobMin, (float) 100.0, "min=1.0 max=200.0 step=1.0 keyIncr=e keyDecr=d" );
//	 mParams.addPersistentParam( "Blob Max R", &mBlobMax, (float) 110.0, "min=1.0 max=200.0 step=1.0 keyIncr=r keyDecr=f" );
//	 mParams.addPersistentParam( "Kinect Tilt", &mKinectTilt, (float) 0.0, "min=-31 max=31 keyIncr=T keyDecr=t" );

    std::cout << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
    
    if (Kinect::getNumDevices()>0){
        mKinect = Kinect::create();
    }

	 mTargetPosition = vec3(0);
}

void HandOpenCVTracker::update(){
    if (!mKinect) return;
	if (mKinect->checkNewDepthFrame()) {

		ImageSourceRef depthSurface = mKinect->getDepthImage();

		// make a texture to display
         mDepthTexture = gl::Texture::create(depthSurface);
		// make a surface for opencv
		mDepthSurface = depthSurface;

        // once the surface is avalable pass it to opencv
        // had trouble here with bit depth. surface comes in full color, needed to crush it down
        cv::Mat input(toOcv(Channel8u(mDepthSurface))), blurred, thresholded, thresholded2, output;

        cv::blur(input, blurred, cv::Size(10, 10));

        // make two thresholded images one to display and one
        // to pass to find contours since its process alters the image
        cv::threshold(blurred, thresholded, mThreshold, 255, CV_THRESH_BINARY);
        cv::threshold(blurred, thresholded2, mThreshold, 255, CV_THRESH_BINARY);

        // 2d vector to store the found contours
        vector<vector<cv::Point> > contours;
        // find em
        cv::findContours(thresholded, contours, CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);

        // convert theshold image to color for output
        // so we can draw blobs on it
        cv::cvtColor(thresholded2, output, CV_GRAY2RGB);

        // loop the stored contours
        for (vector<vector<cv::Point> >::iterator it = contours.begin(); it < contours.end(); it++) {

            // center abd radius for current blob
            cv::Point2f center;
            float radius;
            // convert the cuntour point to a matrix
            vector<cv::Point> pts = *it;
            cv::Mat pointsMatrix = cv::Mat(pts);
            // pass to min enclosing circle to make the blob
            cv::minEnclosingCircle(pointsMatrix, center, radius);

            cv::Scalar color(0, 255, 0);

            if (radius > mBlobMin && radius < mBlobMax) {
                // draw the blob if it's in range
                cv::circle(output, center, radius, color);

                //update the target position
                mTargetPosition.x = 640 - center.x;
                mTargetPosition.y = center.y;
                mTargetPosition.z = 0;
                tracking = true;
            } else {
                tracking = false;
            }
        }

        mCvTexture = gl::Texture::create(fromOcv(output));
	}

	if (mKinect->checkNewVideoFrame())
        mColorTexture = gl::Texture::create(mKinect->getVideoImage());

	if (mKinectTilt != mKinect->getTilt())
		mKinect->setTilt(mKinectTilt);
}

void HandOpenCVTracker::draw()
{


    //	gl::enableDepthWrite();
    //	gl::enableDepthRead();
    gl::pushMatrices();
    	gl::clear(Color(0.0f, 0.0f, 0.0f));

        if (mColorTexture)
            gl::draw(mColorTexture, Rectf(0,0, 100, 100));
        if (mDepthTexture)
            gl::draw(mDepthTexture, Rectf(100, 0, 200, 100));
        if (mCvTexture)
            gl::draw(mCvTexture, Rectf(200, 0, 300, 100));

        if (tracking){
            gl::color(Colorf(1.0f, 0.0f, 0.0f));
            gl::drawSphere(mTargetPosition+vec3(-1280, 920,0), 10.0f, 5);
        }

    gl::popMatrices();
    //	gl::disableDepthWrite();
    //	gl::disableDepthRead();
}

vec3 HandOpenCVTracker::getTargetPosition() const
{
    return mTargetPosition;
}

bool HandOpenCVTracker::isTracking()
{
	return tracking;
}






