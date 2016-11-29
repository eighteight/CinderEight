//
//  CocoaKinectApp.h
//  CocoKinect
//
//  Created by Vladimir Gusev on 6/1/12.
//  Copyright (c) 2012 SMALLAB.ORG. All rights reserved.
//

#ifndef CocoKinect_CocoaKinectApp_h
#define CocoKinect_CocoaKinectApp_h

#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Quaternion.h"
#include "VOpenNIHeaders.h"
#include "OscSender.h"
#include "cinder/Thread.h"
#include <boost/unordered_map.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageSourceKinectColor : public ImageSource
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
    
	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );
        
		for( uint32_t row	 = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width * 3 );
	}
    
protected:
	uint32_t					_width, _height;
	uint8_t						*mData;
    
};


class ImageSourceKinectDepth : public ImageSource
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
    
	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );
        
		for( uint32_t row = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width );
	}
    
protected:
	uint32_t					_width, _height;
	uint16_t					*mData;
};


class CocoKinect : public AppBasic, V::UserListener
{
public:
    
    CocoKinect();
	~CocoKinect();
    
	static const int WIDTH = 379;
	static const int HEIGHT = 279;
    
	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
    static const int KINECT_DEPTH_SIZE = KINECT_DEPTH_HEIGHT*KINECT_DEPTH_WIDTH;
	static const int KINECT_DEPTH_FPS = 30;

    Surface16u originalDepthSurface;
    Surface16u resizedSurface;
    
	void setup();
	void mouseDown( MouseEvent event );
	void update();
	void draw();
    void shutdown();
	void keyDown( KeyEvent event );

    void fileDrop( FileDropEvent event ); 

    boost::unordered_map<int, vector<Vec3f> > userPositions;
    
    void serializeSkeleton(int, vector<Vec3f> positions);
    void reset();
    bool setDevice();
	void onNewUser( V::UserEvent event );
	void onLostUser( V::UserEvent event );
    void toggleDisplayInput();
    void toggleSendOsc();
    void setNumUsers(uint32_t numUsers);
    void setOscAddress(std::string);
    
	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}
    
	ImageSourceRef getUserImage( int id )
	{
		_device0->getLabelMap( id, pixels );
		return ImageSourceRef( new ImageSourceKinectDepth( pixels, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}
    
	void prepareSettings( Settings *settings );
    
private:
    bool isDisplayInput, isSendOsc, isDevice;

    V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;
    
	gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
    std::map<int, gl::Texture> mUsersTexMap;
	
	uint16_t*				pixels;
    uint32_t numUsers;
    
    Font mFont;

    std::string pixStr, pixStrZ, skelStrZ;
    
	Vec3f openniCenter;
    osc::Sender sender;
	std::string host;
    std::string status;
	int port;
};



#endif
