/*
 * Zoetrope.h
 *
 *  Created on: Jan 2, 2012
 *      Author: eight
 */

#ifndef ZOETROPE_H_
#define ZOETROPE_H_

class Flywheel {
public:
	Flywheel();
	virtual ~Flywheel();
	void applyPush(float );
	float getNormalizedLocation();
    float getSpeed() const;
    void setSpeed(float);
    float getMaxLocation() const;
    void setMaxLocation(float maxLocation);
    float getLocation() const;
    void setLocation(float location);
    void setSkipLocation(float skipLocation);
private:
    float frictionCoefficient;
    float mass;
    float deltaT;
    float speed;
    float location;
    float maxLocation;
};

#endif /* ZOETROPE_H_ */
