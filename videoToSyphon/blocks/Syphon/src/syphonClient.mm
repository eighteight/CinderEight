/*
 *  syphonServer.cpp
 *  syphonTest
 *
 *  Created by astellato on 11/6/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "syphonClient.h"

#import <Syphon/Syphon.h>
#import "SyphonNameboundClient.h"

syphonClient::syphonClient()
{
	bSetup = false;
}

syphonClient::~syphonClient()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
    [(SyphonNameboundClient*)mClient release];
    mClient = nil;
    
    [pool drain];
}

void syphonClient::setup()
{
    // Need pool
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	mClient = [[SyphonNameboundClient alloc] init]; 
	
	bSetup = true;
    
    [pool drain];
}

void syphonClient::setApplicationName(std::string appName)
{
    if(bSetup)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:appName.c_str() encoding:[NSString defaultCStringEncoding]];
        
        [(SyphonNameboundClient*)mClient setAppName:name];
		
        [pool drain];
    }
    
}
void syphonClient::setServerName(std::string serverName)
{
    if(bSetup)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        NSString *name = [NSString stringWithCString:serverName.c_str() encoding:[NSString defaultCStringEncoding]];
		
        if([name length] == 0)
            name = nil;
        
        [(SyphonNameboundClient*)mClient setName:name];
		
        [pool drain];
    }    
}

void syphonClient::bind()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
    if(bSetup)
    {
     	[(SyphonNameboundClient*)mClient lockClient];
        SyphonClient *client = [(SyphonNameboundClient*)mClient client];
		
        latestImage = [client newFrameImageForContext:CGLGetCurrentContext()];
		NSSize texSize = [(SyphonImage*)latestImage textureSize];
		
		/* to do
        // we now have to manually make our ofTexture's ofTextureData a proxy to our SyphonImage
        mTex.texData.textureID = [(SyphonImage*)latestImage textureName];
        mTex.texData.textureTarget = GL_TEXTURE_RECTANGLE_ARB;  // Syphon always outputs rect textures.
        mTex.texData.width = texSize.width;
        mTex.texData.height = texSize.height;
        mTex.texData.tex_w = texSize.width;
        mTex.texData.tex_h = texSize.height;
        mTex.texData.tex_t = texSize.width;
        mTex.texData.tex_u = texSize.height;
        mTex.texData.glType = GL_RGBA;
        mTex.texData.pixelType = GL_UNSIGNED_BYTE;
        mTex.texData.bFlipTexture = YES;
        mTex.texData.bAllocated = YES;
        
        mTex.bind();
		 */
		
		mTex = ci::gl::Texture(GL_TEXTURE_RECTANGLE_ARB, [(SyphonImage*)latestImage textureName],
								   texSize.width, texSize.height, false);
		
		mTex.bind();
    }
    else
		std::cout<<"syphonClient is not setup, or is not properly connected to server.  Cannot bind.\n";
    
    [pool drain];
}

void syphonClient::unbind()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    if(bSetup)
    {
        mTex.unbind();
		
        [(SyphonNameboundClient*)mClient unlockClient];
        [(SyphonImage*)latestImage release];
        latestImage = nil;
    }
    else
		std::cout<<"syphonClient is not setup, or is not properly connected to server.  Cannot unbind.\n";
	
	[pool drain];
}

void syphonClient::draw(ci::Vec2f origin, ci::Vec2f drawSize){
	draw(origin.x, origin.y, drawSize.x, drawSize.y);
}

void syphonClient::draw(float x, float y, float w, float h)
{
	if(bSetup && mTex){
		this->bind();
		
		//mTex.draw(x, y, w, h);
		ci::gl::drawSolidRect(ci::Rectf(x, y, x + w, y + h));
		
		this->unbind();
	}
}

void syphonClient::draw(float x, float y)
{
	if(bSetup && mTex){
		//this->draw(x,y, mTex.width, mTex.height);
		ci::gl::draw(mTex,ci::Vec2f(x, y));
	}
}
