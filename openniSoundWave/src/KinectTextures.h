//
//  KinectTextures.h
//  OpenniStream
//
//  Created by Vladimir Gusev on 6/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OpenniStream_KinectTextures_h
#define OpenniStream_KinectTextures_h
#include "cinder/ImageIo.h"

class ImageSourceKinectColor : public cinder::ImageSource 
{
public:
	ImageSourceKinectColor( uint8_t *buffer, int width, int height )
    : ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_RGB );
		setChannelOrder( ImageIo::RGB );
		setDataType( ImageIo::UINT8 );
	}
    
	~ImageSourceKinectColor()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
         delete[] mData;
         mData = NULL;
         }*/
	}
    
	virtual void load( cinder::ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );
        
		for( uint32_t row	 = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width * 3 );
	}
    
protected:
	uint32_t					_width, _height;
	uint8_t						*mData;
    
};


class ImageSourceKinectDepth : public cinder::ImageSource 
{
public:
	ImageSourceKinectDepth( uint16_t *buffer, int width, int height )
    : ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_GRAY );
		setChannelOrder( ImageIo::Y );
		setDataType( ImageIo::UINT16 );
	}
    
	~ImageSourceKinectDepth()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
         delete[] mData;
         mData = NULL;
         }*/
	}
    
	virtual void load( cinder::ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );
        
		for( uint32_t row = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width );
	}
    
protected:
	uint32_t					_width, _height;
	uint16_t					*mData;
};


#endif
