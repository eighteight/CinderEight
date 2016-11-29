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

#include "ThreadedImage.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

ThreadedImage::ThreadedImage(void)
{
	mSurfaces.clear();
	mTextures.clear();

	// TODO: maximum texture size of videocard should be queried from OpenGL
	mMaxSize = 2048;
}

ThreadedImage::~ThreadedImage(void)
{
	// gracefully abort current threads:
	deque<shared_ptr<thread>>::iterator itr;
	// tell all threads to stop as soon as possible
	for(itr=mThreads.begin();itr!=mThreads.end();++itr)
		(*itr)->interrupt();
	// wait for all threads to finish
	for(itr=mThreads.begin();itr!=mThreads.end();++itr)
		(*itr)->join();

	mThreads.clear();
}

void ThreadedImage::setup()
{
}

void ThreadedImage::update()
{
	// remove finished threads
	deque<shared_ptr<thread>>::iterator itr;
	for(itr=mThreads.begin();itr!=mThreads.end();)
	{
		if( (*itr)->timed_join<posix_time::milliseconds>(posix_time::milliseconds(1)) )
			itr = mThreads.erase(itr);
		else
			++itr;
	}

	// create texture from finished surfaces
	mSurfaceMutex.lock();
	if(!mSurfaces.empty())
	{
		// destroy existing texture(s)
		mTextures.clear();

		// create texture from most recently loaded surface only
		mTextures.push_back( gl::Texture(mSurfaces.back()) );
		mSurfaces.clear();
	}
	mSurfaceMutex.unlock();
}

void ThreadedImage::draw()
{
	// draw texture if it exists
	if(!mTextures.empty())
	{
		gl::color( Color::white() );
		gl::draw(mTextures.front());
	}

	// draw spinning wait cursor while loading
	if(!mThreads.empty())
	{
		int segments = 10;
		float radius = 25.0f;
		float size = 5.0f;
		float alpha = (float) getElapsedSeconds() - (int) getElapsedSeconds();
		Vec2f center = 0.5f * Vec2f(getSize());

		gl::enableAlphaBlending();
		for(int i=0;i<segments;++i)
		{
			float angle = i * toRadians(-360.0f / segments);
			Vec2f pt = center + radius * Vec2f( cosf(angle), sinf(angle) );

			gl::color( ColorAf(1.0f, 1.0f, 1.0f, 1.0f - alpha) );
			gl::drawSolidCircle(pt, size);

			alpha += 1.0f / segments;
			if(alpha > 1.0f) alpha -= 1.0f;
		}
		gl::disableAlphaBlending();
	}
}

void ThreadedImage::load(const string &url)
{
	// terminate existing threads
	// (thread will be flagged as 'terminated' so it will not pass back its Surface to the main thread)
	deque<shared_ptr<thread>>::iterator itr;
	for(itr=mThreads.begin();itr!=mThreads.end();++itr)
		(*itr)->interrupt();

	// create thread
	console() << getElapsedSeconds() << ":" << "Creating thread..." << endl;
	mThreads.push_back( shared_ptr<thread>(new thread(&ThreadedImage::threadLoad, this, url)) );
}

Area ThreadedImage::getBounds()
{
	if(mTextures.empty()) return Area(0,0,100,100);
	else return mTextures.front().getBounds();
}

Vec2i ThreadedImage::getSize()
{
	if(mTextures.empty()) return Vec2i(100,100);
	else return mTextures.front().getSize();
}

void ThreadedImage::threadLoad(const string &url)
{
	Surface surface;
	ImageSourceRef image;

	console() << getElapsedSeconds() << ":" << "Loading:" << url << endl;

	try { 
		// try to load from FILE
		image = ci::loadImage( ci::loadFile( url ) ); 
		console() << getElapsedSeconds() << ":" << "File loaded successfully:" << url << endl;
	}
	catch(...) { 
		try {
			// try to load from URL
			image = ci::loadImage( ci::loadUrl( Url(url) ) ); 
			console() << getElapsedSeconds() << ":" << "Url loaded successfully:" << url << endl;
		}
		catch(...) {
			// both attempts to load the url failed
			console() << getElapsedSeconds() << ":" << "Failed to load:" << url << endl;
			return;
		}
	}

	// allow interruption now (robust version: catch the exception and deal with it)
	console() << getElapsedSeconds() << ":" << "Checking for interruption..." << endl;
	try { 
		boost::this_thread::interruption_point();
	}
	catch(boost::thread_interrupted) {
		console() << getElapsedSeconds() << ":" << "Interruption detected, exiting..." << endl;

		// exit the thread
		return;
	}
	console() << getElapsedSeconds() << ":" << "No interruption detected" << endl;

	// create surface
	console() << getElapsedSeconds() << ":" << "Creating surface..." << endl;
	surface = Surface(image);
	console() << getElapsedSeconds() << ":" << "Surface created" << endl;

	// allow interruption now (lazy version: exit the thread because of an unhandled exception)
	console() << getElapsedSeconds() << ":" << "Checking for interruption..." << endl;
	boost::this_thread::interruption_point();
	console() << getElapsedSeconds() << ":" << "No interruption detected" << endl;

	// resize
	Area source = surface.getBounds();
	Area dest(0, 0, mMaxSize, mMaxSize);
	Area fit = Area::proportionalFit(source, dest, false, false);
		
	if(source.getSize() != fit.getSize())
	{
		console() << getElapsedSeconds() << ":" << "Resizing surface..." << endl;
		surface = ip::resizeCopy(surface, source, fit.getSize());
		console() << getElapsedSeconds() << ":" << "Surface resized" << endl;
	}

	// allow interruption now
	console() << getElapsedSeconds() << ":" << "Checking for interruption..." << endl;
	boost::this_thread::interruption_point();
	console() << getElapsedSeconds() << ":" << "No interruption detected" << endl;

	// copy to main thread
	mSurfaceMutex.lock();
	mSurfaces.push_back(surface);
	mSurfaceMutex.unlock();

	console() << getElapsedSeconds() << ":" << "Thread finished" << endl;
}
