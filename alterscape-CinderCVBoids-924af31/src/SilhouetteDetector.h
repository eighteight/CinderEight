/*
 *  SilhouetteDetector.h
 *  Boids
 *
 *  Created by Ryan Spicer on 12/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
#include "cinder/Vector.h"
#include "cinder/Utilities.h"
#include "cinder/Surface.h"
#include "CinderOpenCV.h"
#include <vector>


typedef boost::shared_ptr<ci::Vec2i> Vec2i_ptr;
typedef boost::shared_ptr<std::vector<Vec2i_ptr> > Vec2i_ptr_vec;

class SilhouetteDetector {
public:
	SilhouetteDetector(int width, int height);
	~SilhouetteDetector();
	void processSurface(ci::Surface8u *captureSurface, std::vector<Vec2i_ptr_vec> *polygons, ci::Surface8u *processedOutput);
	int cvThresholdLevel;
private:
	IplImage *silhouette;
	IplImage *img_8uc3;
	cv::Mat input, gray, output;
	std::vector<Vec2i_ptr> points;
	
};