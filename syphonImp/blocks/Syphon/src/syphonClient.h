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

class syphonClient {
	public:
	syphonClient();
	~syphonClient();
	
	void setup();
	
	void setApplicationName(std::string appName);
    void setServerName(std::string serverName);
	
    void bind();
    void unbind();
    
	void draw(ci::Vec2f origin, ci::Vec2f drawSize);
    void draw(float x, float y, float w, float h);
    void draw(float x, float y);
    
protected:
	void* mClient;
    void* latestImage;
	ci::gl::Texture mTex;
	int width, height;
	bool bSetup;
	std::string name;
};