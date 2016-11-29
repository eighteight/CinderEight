/*
 *  syphonServer.h
 *  syphonTest
 *
 *  Created by astellato on 11/6/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"

class syphonServer {
	public:
	syphonServer();
	~syphonServer();
	void setName (std::string n);
	std::string getName();
	void publishScreen();
    void publishTexture(ci::gl::Texture* inputTexture);
	
    
protected:
	void *mSyphon;

};