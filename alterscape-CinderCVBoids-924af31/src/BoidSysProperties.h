/*
 *  BoidSysProperties.h
 *  Boids
 *
 *  Created by Joanna Merson on 12/6/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

struct BoidSysProperties {
	float	zoneRadius;
	float	lowerThresh;
	float	higherThresh;
	float	attractStrength;
	float	repelStrength;
	float	orientStrength;
	float	silThresh;
	float	silRepelStrength;

	bool	centralGravity;
	bool	flatten;
	bool	gravity;

	ci::Color baseColor;

};

struct BoidSysPair {
	BoidSysProperties flockOneProps;
	BoidSysProperties flockTwoProps;
	ci::Color imageColor;
};