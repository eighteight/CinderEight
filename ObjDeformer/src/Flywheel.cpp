/*
 * Zoetrope.cpp
 *
 *  Created on: Jan 2, 2012
 *      Author: eight
 */

#include "Flywheel.h"
#include <iostream>

using namespace std;

Flywheel::Flywheel() {
	speed = 0.0f;
	frictionCoefficient = 0.1f;
	mass = 500.0f;
	deltaT = 1.0f;
	location = 0.0f;
}

Flywheel::~Flywheel() {
}

void Flywheel::applyPush(float pushSpeed)
{
	float force = (pushSpeed - speed)*frictionCoefficient;
	float accel = force/mass;
	speed = speed + accel*deltaT;
    location += speed*deltaT;
}

float Flywheel::getSpeed() const
{
    return speed;
}

void Flywheel::setSpeed(float speed)
{
    this->speed = speed;
}

float Flywheel::getMaxLocation() const
{
    return maxLocation;
}

void Flywheel::setMaxLocation(float maxLocation)
{
    this->maxLocation = maxLocation;
}

float Flywheel::getLocation() const
{
    return location;
}

void Flywheel::setLocation(float location)
{
    this->location = location;
}

















