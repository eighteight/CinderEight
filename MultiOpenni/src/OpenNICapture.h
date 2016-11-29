#pragma once

#include <XnCppWrapper.h>
#include <XnCodecIDs.h>
#include <stdio.h>
#include <string>
#include "_2RealKinect.h"

typedef struct NodeCapturingData
{
	XnCodecID captureFormat;
	XnUInt32 nCapturedFrames;
	bool bRecording;
	xn::Generator* pGenerator;
} NodeCapturingData;

#define MAX_STRINGS 20

typedef struct
{
	int nValuesCount;
	XnCodecID pValues[MAX_STRINGS];
    std::string pIndexToName[MAX_STRINGS];
} NodeCodec;

class OpenNICapture{

public:
	OpenNICapture();
	bool setup(_2RealKinectWrapper::_2RealKinect* kinect, uint devNum, XnCodecID depthFormat=XN_CODEC_16Z_EMB_TABLES, XnCodecID imageFormat=XN_CODEC_JPEG, XnCodecID irFormat=XN_CODEC_NULL, XnCodecID audioFormat=XN_CODEC_NULL);
	bool startCapture();
	void stopCapture();

	void update();

	static std::string LOG_NAME;

private:

	XnStatus captureFrame();

	// --------------------------------
	// Types
	// --------------------------------
	enum CapturingState	{
		NOT_CAPTURING,
		SHOULD_CAPTURE,
		CAPTURING,
	};

	enum CaptureNodeType
	{
		CAPTURE_DEPTH_NODE,
		CAPTURE_IMAGE_NODE,
		CAPTURE_IR_NODE,
		CAPTURE_AUDIO_NODE,
		CAPTURE_NODE_COUNT
	};

	NodeCapturingData nodes[CAPTURE_NODE_COUNT];
	xn::Recorder* pRecorder;
    _2RealKinectWrapper::OpenNIDevice* device;

	XnUInt32 nStartOn; // time to start, in seconds
	bool bSkipFirstFrame;
	CapturingState State;
	XnUInt32 nCapturedFrameUniqueID;
	std::string csDisplayMessage;

	NodeCodec g_DepthFormat;
	NodeCodec g_ImageFormat;
	NodeCodec g_IRFormat;
	NodeCodec g_AudioFormat;

	xn::Context * context;
    uint numDevice;

};
