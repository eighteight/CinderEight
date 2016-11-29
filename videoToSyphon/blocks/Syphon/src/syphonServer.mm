/*
 *  syphonServer.cpp
 *  syphonTest
 *
 *  Created by astellato on 11/6/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "syphonServer.h"

#import <Syphon/Syphon.h>

syphonServer::syphonServer()
{
	mSyphon = nil;
}

syphonServer::~syphonServer()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
    [(SyphonServer *)mSyphon stop];
    [(SyphonServer *)mSyphon release];
    
    [pool drain];
}


void syphonServer::setName(std::string n)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
	NSString *title = [NSString stringWithCString:n.c_str() 
										 encoding:[NSString defaultCStringEncoding]];
	
	if (!mSyphon)
	{
		mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
	}
	else
	{
		[(SyphonServer *)mSyphon setName:title];
	}
    
    [pool drain];
}

std::string syphonServer::getName()
{
	std::string name;
	if (mSyphon)
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
		name = [[(SyphonServer *)mSyphon name] cStringUsingEncoding:[NSString defaultCStringEncoding]];
		
		[pool drain];
	}
	else
	{
		name = "Untitled";
	}
	return name;
}

void syphonServer::publishScreen()
{
	ci::gl::Texture mTex =  ci::gl::Texture(ci::app::copyWindowSurface());
	this->publishTexture(&mTex);
	
	/*
	int w = ofGetWidth();
	int h = ofGetHeight();
	
	ofTexture tex;
	tex.allocate(w, h, GL_RGBA);
	
	tex.loadScreenData(0, 0, w, h);
	
	this->publishTexture(&tex);
	
	tex.clear();
	 */
}


void syphonServer::publishTexture(ci::gl::Texture* inputTexture)
{
	if(inputTexture){
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		GLuint texID = inputTexture->getId();
		if (!mSyphon)
		{
			mSyphon = [[SyphonServer alloc] initWithName:@"Untitled" context:CGLGetCurrentContext() options:nil];
		}
		[(SyphonServer *)mSyphon publishFrameTexture:texID textureTarget:GL_TEXTURE_2D imageRegion:NSMakeRect(0, 0, inputTexture->getWidth(), inputTexture->getHeight()) textureDimensions:NSMakeSize(inputTexture->getWidth(), inputTexture->getHeight()) flipped:false];
		[pool drain];
	} else {
		ci::app::console()<<"syphonServer is not setup, or texture is not properly backed.  Cannot draw.\n";
	}
	
	/*
    // If we are setup, and our input texture
	if(inputTexture->bAllocated())
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
		ofTextureData texData = inputTexture->getTextureData();
		
		if (!mSyphon)
		{
			mSyphon = [[SyphonServer alloc] initWithName:@"Untitled" context:CGLGetCurrentContext() options:nil];
		}
		
		[(SyphonServer *)mSyphon publishFrameTexture:texData.textureID textureTarget:texData.textureTarget imageRegion:NSMakeRect(0, 0, texData.width, texData.height) textureDimensions:NSMakeSize(texData.width, texData.height) flipped:!texData.bFlipTexture];
        [pool drain];
    } 
    else 
    {
		cout<<"syphonServer texture is not properly backed.  Cannot draw.\n";
	}
	 */
}

