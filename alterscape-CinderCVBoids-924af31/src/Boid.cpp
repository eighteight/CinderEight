#include "Boid.h"
#include "BoidController.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using std::vector;

Boid::Boid( Vec3f pos, Vec3f vel, bool followed, BoidController* parent )
{
	this->parent	= parent;
	this->pos		= pos;
	tailPos			= pos;
	this->vel		= vel;
	velNormal		= Vec3f::yAxis();
	this->acc		= Vec3f::zero();
	radius			= Rand::randFloat( 15.0f, 23.0f );
	
	mNeighborPos	= Vec3f::zero();
	mNumNeighbors	= 0;
	mMaxSpeed		= Rand::randFloat( 2.5f, 4.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
	
	mColor			= ColorA( 0.0f, 0.0f, 0.0f, 0.0f );
	this->mGravity	= parent->getGravity(this);

	
	mDecay			= 0.99f;
	mRadius			= 1.0f;
	mLength			= 5.0f;
	mFear			= 1.0f;
	mCrowdFactor	= 1.0f;
	
	mIsDead			= false;
	mFollowed		= followed;
	drawClosestSilhouettePoint = true;
	
	
	
	// ** wing code ** //
	//note: 'aLoc' in Hodgin is 'pos' here.
	
	mLen			= 15;
	mInvLen			= 1.0f / (float)mLen;
	for( int i=0; i<mLen; ++i ) {
		mLoc.push_back( pos );
	}
	// ** end wing code ** //
	
	
	
}

void Boid::pullToCenter( const Vec3f &center )
{
	Vec3f dirToCenter = pos - center;
	float distToCenter = dirToCenter.length();
	float distThresh = 200.0f;
	
	if( distToCenter > distThresh ){
		dirToCenter.normalize();
		float pullStrength = 0.00025f;
		vel -= dirToCenter * ( ( distToCenter - distThresh ) * pullStrength );
	}
}


void Boid::update(bool flatten)
{	
	mColor = parent->getColor(this);
	mGravity = parent->getGravity(this);
	//std::cout<< "gravity is: " << mGravity << std::endl;
	
	mCrowdFactor -= ( mCrowdFactor - ( 1.0f - mNumNeighbors * 0.02f ) ) * 0.1f;
	mCrowdFactor = constrain( mCrowdFactor, 0.5f, 1.0f );
	
	mFear -= ( mFear) * 0.2f;
	
	if( flatten )
	{
		acc.z = 0.0f;
	}
	
	if( mGravity )
	{
		acc.y = acc.y-0.2f;//add accelleration due to'gravity'
		
		//test if the boid is out the bottom
		if (pos.y < -0.3*(float)app::getWindowHeight()) { 
			//bounce it back upwards, but lose energy  in the bounce
			if (vel.y < 0.0f) {
				vel.y= -.75f*vel.y;
			}
			if(acc.y < 0.0f) {
				acc.y= -0.75f*acc.y;
			}
		}
	}
	vel += acc;
	velNormal = vel.normalized();
	
	limitSpeed();
	
	
	pos += vel;
	if( flatten ){
		pos.z = 0.0f;
	}
	
	tailPos = pos - ( velNormal * mLength );
	vel *= mDecay;
	
	//float c = math<float>::min( mNumNeighbors/50.0f, 1.0f );
//	mColor = ColorA( CM_HSV, 1.0f - c, c, c * 0.5f + 0.5f, 1.0f );
	

	
	acc = Vec3f::zero();
	mNeighborPos = Vec3f::zero();
	mNumNeighbors = 0;
	
	// ** trail code ** //
	
	//update position array
	for( int i=mLen-1; i>0; i-- ) {
		mLoc[i] = mLoc[i-1];
	}
	mLoc[0] += vel;
	
	if( flatten ){
		mLoc[0].z=0.0;
	}
	//update 
	
	// ** end trail code ** //
}

void Boid::limitSpeed()
{
	float maxSpeed = mMaxSpeed + mCrowdFactor;
	float maxSpeedSqrd = maxSpeed * maxSpeed;
	
	float vLengthSqrd = vel.lengthSquared();
	if( vLengthSqrd > maxSpeedSqrd ){
		vel = velNormal * maxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		vel = velNormal * mMinSpeed;
	}
	vel *= (1.0 + mFear );
}

void Boid::draw()
{

	
	glDepthMask( GL_FALSE ); //IMPORTANT
	glDisable( GL_DEPTH_TEST ); //IMPORTANT
	glEnable( GL_BLEND ); //IMPORTANT
	glBlendFunc( GL_SRC_ALPHA, GL_ONE ); //IMPORTANT
	
	
	glPushMatrix();
	glTranslatef( pos.x, pos.y, 0 );
	glScalef( radius, radius, radius );
	glColor4f(mColor);
	glBegin( GL_QUADS );
	glTexCoord2f(0, 0);    glVertex2f(-.5, -.5);
	glTexCoord2f(1, 0);    glVertex2f( .5, -.5);
	glTexCoord2f(1, 1);    glVertex2f( .5,  .5);
	glTexCoord2f(0, 1);    glVertex2f(-.5,  .5);
	glEnd();
	glPopMatrix();
	
	if(drawClosestSilhouettePoint) {
		glColor4f(1.0f,1.0f,0.0f,1.0f);
		glBegin(GL_LINES);
		glLineWidth(1.0f);
		gl::vertex(pos);
		gl::vertex(closestSilhouettePoint);
		glEnd();
	}
}

void Boid::addNeighborPos( Vec3f pos )
{
	mNeighborPos += pos;
	mNumNeighbors ++;
}

// ** trail code ** //
void Boid::renderQuadStripTrail()
{
	glDisable( GL_TEXTURE_2D );

	
	gl::color( mColor );
	glBegin( GL_QUAD_STRIP );
	for( int i=0; i<mLen-2; i++ ){
		float per	= i / (float)(mLen-1);
		
		Vec3f perp0	= Vec3f( mLoc[i].x, mLoc[i].y, 0.0f ) - Vec3f( mLoc[i+1].x, mLoc[i+1].y, 0.0f );
		Vec3f perp1	= perp0.cross( Vec3f::zAxis() );
		Vec3f perp2	= perp0.cross( perp1 );
		perp1	= perp0.cross( perp2 ).normalized();
		
		Vec3f off	= perp1 * ( radius * ( 1.0f - per )  ); // controls trail width
		
		glColor4f( ( 1.0f - per ) * 0.5f, 0.15f, per * 0.5f, ( 1.0f - per ) * 0.25f );
		glVertex3fv( mLoc[i] - off );
		glVertex3fv( mLoc[i] + off );
	}
	
	glEnd();
	
	glEnable( GL_TEXTURE_2D );

}
// ** trail code ** //