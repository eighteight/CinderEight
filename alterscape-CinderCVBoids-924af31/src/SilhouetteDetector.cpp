/*
 *  SilhouetteDetector.cpp
 *  Boids
 *
 *  Created by Ryan Spicer on 12/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "SilhouetteDetector.h"

using namespace std;
using namespace boost;
using namespace ci;

SilhouetteDetector::SilhouetteDetector(int width, int height) {
	cvThresholdLevel = 40;
	img_8uc3 = cvCreateImage( cvSize(width,height), 8, 3 );	
	silhouette = cvCreateImageHeader(cvSize(width,height), 8, 1);	
}

void SilhouetteDetector::processSurface(ci::Surface8u* surface, vector<Vec2i_ptr_vec> *polygons, ci::Surface8u *processedOutput) {
	
	input = toOcv( *surface );		//define cv::Mat -- CV Matrices that we'll need
	cv::cvtColor(input,gray,CV_RGB2GRAY);								//convert the input to greyscale, stick it in 'grey'
	cv::threshold( gray, output, cvThresholdLevel, 255, CV_8U );		//threshhold the input (make it b&w)
	cv::Mat dilated;
	cv::Mat erroded;
	cv::dilate(output,dilated,cv::Mat());
	cv::erode(dilated,erroded,cv::Mat());
	
	ci::Surface outputSurface = fromOcv(erroded);
	//processedOutput->setData(outputSurface.getData(),outputSurface.getWidth(),outputSurface.getHeight(),outputSurface.getRowBytes());
	processedOutput->copyFrom( outputSurface, outputSurface.getBounds() );
	
	//HERE BE DRAGONS
	silhouette->imageData = (char*)erroded.data;			//here we're switching to the older-but-more-capable OpenCV C API, so make an IplImage
	//create pointers to store data we're going to be calculating
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* first_contour = NULL;
	CvSeq* first_polygon = NULL;
	
	//find the contours (edges) of the silhouette, in terms of pixels.
	cvFindContours( silhouette,
				   storage,
				   &first_contour,
				   sizeof(CvContour),
				   CV_RETR_LIST );
	
	//convert the pixel contours to line segments in a polygon.
	first_polygon = cvApproxPoly(first_contour, 
								 sizeof(CvContour), 
								 storage,
								 CV_POLY_APPROX_DP, 
								 2.0,
								 1);
	
	//for each polygon
	for ( CvSeq* c=first_polygon; c!=NULL; c=c->h_next ) {
		//skip polygons containing less than 5 points.
		if (c->total < 5)
			continue;
		
		Vec2i_ptr_vec polyPoints = boost::shared_ptr<vector<Vec2i_ptr> >(new vector<Vec2i_ptr>());
		for (int i=0;i<c->total;i++) {
			CvPoint * point = CV_GET_SEQ_ELEM(CvPoint,c,i);
			Vec2i_ptr point_vec = boost::shared_ptr<Vec2i>(new Vec2i(point->x,point->y));
			polyPoints.get()->push_back(point_vec);
		}
		polygons->push_back(polyPoints);
	}
	
	//This is sort of absurd, but it lets me pass an output back through
	cvReleaseMemStorage(&storage);

}

SilhouetteDetector::~SilhouetteDetector() {
	cvReleaseImage(&img_8uc3);
	cvReleaseImage(&silhouette);
}