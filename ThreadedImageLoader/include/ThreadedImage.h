/*
 Copyright (C)2010 Paul Houx
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/ip/Resize.h"
#include "cinder/ImageIo.h"
#include "cinder/Thread.h"
#include "cinder/Url.h"

#include <deque>

class ThreadedImage
{
public:
	ThreadedImage(void);
	virtual ~ThreadedImage(void);

	void setup();
	void update();
	void draw();

	void load(const std::string &url);

	ci::Area	getBounds();
	ci::Vec2i	getSize();
protected:
	// note: we use 'deque' instead of 'vector' for easier access to the first element
	std::deque<ci::Surface>		mSurfaces;
	std::deque<ci::gl::Texture>	mTextures;

	boost::mutex	mSurfaceMutex;
	std::deque<boost::shared_ptr<boost::thread>>	mThreads;

	GLint	mMaxSize;

	void threadLoad(const std::string &url);
};
