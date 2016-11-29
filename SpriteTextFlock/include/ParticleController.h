#pragma once
#include "Particle.h"
#include "cinder/gl/TextureFont.h"
#include <list>

class ParticleController {
 public:
	ParticleController();
	void applyForce( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float orientStrength );
	void pullToCenter( const ci::Vec3f &center );
	void update( bool flatten, std::string* currentChar  );
	void draw();
	void draw(cinder::gl::TextureFontRef* textureFont);
	void addParticles( int amt );
	void removeParticles( int amt );

	std::list<Particle>	mParticles;
	ci::Vec3f mParticleCentroid;
	int mNumParticles;
};
