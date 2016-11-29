//
//  gpGpuFrameBuffer.h
//  dsFluid
//
//  Created by luca lolli on 16/12/2015.
//
//

#include "gpGpuFrameBuffer.h"

using namespace ci;
using namespace ci::app;
using namespace std;



gpGpuFrameBuffer::gpGpuFrameBuffer(int width, int height, GLint colorFormat):mWidth(width), mHeight(height), mColorFormat(colorFormat)
{
    initFbo();
}

gpGpuFrameBuffer::gpGpuFrameBuffer(ivec2 size, GLint colorFormat )
{
    
    gpGpuFrameBuffer(size.x, size.y, colorFormat);
    
}

gpGpuFrameBuffer::~gpGpuFrameBuffer(){}

void gpGpuFrameBuffer::initFbo()
{
    mTextureFormat.setMagFilter( GL_NEAREST );
    mTextureFormat.setMinFilter( GL_NEAREST );
    mTextureFormat.setWrap(GL_REPEAT, GL_REPEAT);
    mTextureFormat.setInternalFormat( mColorFormat );
    
    //Create fbo with two attachments for faster ping pong
    mFboFormat.attachment( GL_COLOR_ATTACHMENT0, gl::Texture2d::create( mWidth, mHeight, mTextureFormat ) )
    .attachment( GL_COLOR_ATTACHMENT1, gl::Texture2d::create( mWidth, mHeight, mTextureFormat ) );
    
    mReadIndex	= GL_COLOR_ATTACHMENT0;
    mWriteIndex = GL_COLOR_ATTACHMENT1;
    mFbo		= gl::Fbo::create(mWidth, mHeight, mFboFormat );
    
    clear();
}

void gpGpuFrameBuffer::clear()
{
    mFbo->bindFramebuffer();
    
    const GLenum buffers[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    
    gl::drawBuffers( 2, buffers );
    
    gl::viewport( mFbo->getSize() );
    
    gl::clear(Color(0.0,0.0,0.0));
    
    mFbo->unbindFramebuffer();
}

gl::FboRef gpGpuFrameBuffer::getBuffer()
{
    return mFbo;
}

GLint gpGpuFrameBuffer::getBufferLocation()
{
    return mWriteIndex;
}

GLint gpGpuFrameBuffer::getTextureLocation()
{
    return mWriteIndex;
}

void gpGpuFrameBuffer::drawBuffer()
{
    gl::drawBuffer( mWriteIndex );
}

void gpGpuFrameBuffer::bindTexture()
{
    mFbo->getTexture2d(mReadIndex)->bind();
}

void gpGpuFrameBuffer::unbindTexture()
{
    mFbo->getTexture2d(mReadIndex)->unbind();
}

void gpGpuFrameBuffer::bindBuffer()
{
    mFbo->bindFramebuffer();
    gl::drawBuffer( mWriteIndex );
}

void gpGpuFrameBuffer::unbindBuffer(bool toSwap)
{
    mFbo->unbindFramebuffer();
    
    if(toSwap)
    {
        swap();
    }
}


gl::TextureRef gpGpuFrameBuffer::getTexture()
{
    return mFbo->getTexture2d(mReadIndex);
}

void gpGpuFrameBuffer::swap()
{
    std::swap(mReadIndex,mWriteIndex);
}

void gpGpuFrameBuffer::draw()
{
    gl::draw( mFbo->getTexture2d( mReadIndex ) );
}

ivec2 gpGpuFrameBuffer::getSize()
{
    return ivec2(mWidth, mHeight);
}
