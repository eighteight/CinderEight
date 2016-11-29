/*
 * ofxOpenNICapture.cpp
 *
 *  Created on: 11/10/2011
 *      Author: arturo
 */

#include "OpenNICapture.h"
#include "OpenNIDevice.h"
#include <iostream>

using namespace xn;

std::string OpenNICapture::LOG_NAME = "ofxOpenNICapture";

#define START_CAPTURE_CHECK_RC(rc, what)												\
	if (nRetVal != XN_STATUS_OK)														\
	{																					\
		std::cout << "Failed to" << what << xnGetStatusString(rc);				\
		delete pRecorder;														\
		pRecorder = NULL;														\
		return false;																	\
	}


//----------------------------------------
OpenNICapture::OpenNICapture(){

	// Depth Formats
	int nIndex = 0;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_16Z_EMB_TABLES;
	g_DepthFormat.pIndexToName[nIndex] = "PS Compression (16z ET)";
	nIndex++;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_DepthFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_DepthFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_DepthFormat.nValuesCount = nIndex;

	// Image Formats
	nIndex = 0;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_JPEG;
	g_ImageFormat.pIndexToName[nIndex] = "JPEG";
	nIndex++;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_ImageFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_ImageFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_ImageFormat.nValuesCount = nIndex;

	// IR Formats
	nIndex = 0;

	g_IRFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_IRFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_IRFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_IRFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_IRFormat.nValuesCount = nIndex;

	// Audio Formats
	nIndex = 0;

	g_AudioFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_AudioFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_AudioFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_AudioFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_AudioFormat.nValuesCount = nIndex;

	// Init

	State = NOT_CAPTURING;
	nCapturedFrameUniqueID = 0;
	csDisplayMessage[0] = '\0';
	bSkipFirstFrame = false;

	nodes[CAPTURE_DEPTH_NODE].captureFormat = XN_CODEC_16Z_EMB_TABLES;
	nodes[CAPTURE_IMAGE_NODE].captureFormat = XN_CODEC_JPEG;
	nodes[CAPTURE_IR_NODE].captureFormat = XN_CODEC_NULL;
	nodes[CAPTURE_AUDIO_NODE].captureFormat = XN_CODEC_NULL;

	pRecorder = NULL;
}

//----------------------------------------
bool OpenNICapture::setup(_2RealKinectWrapper::_2RealKinect* _kinect, uint devNum, XnCodecID depthFormat, XnCodecID imageFormat, XnCodecID irFormat, XnCodecID audioFormat){
	context = &(_kinect->getDevices()[devNum]->m_Context);
    device = _kinect->getDevices()[devNum].get();
    numDevice = devNum;
    
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    XnChar strFileName[XN_FILE_MAX_PATH];
    sprintf(strFileName, "%s/%04d%02d%02d-%02d%02d%02d-dev%d.oni", ".",
			timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, numDevice);
    
    std::cout<<strFileName<<std::endl;

	nodes[CAPTURE_DEPTH_NODE].captureFormat = depthFormat;
	nodes[CAPTURE_IMAGE_NODE].captureFormat = imageFormat;
	nodes[CAPTURE_IR_NODE].captureFormat = irFormat;
	nodes[CAPTURE_AUDIO_NODE].captureFormat = audioFormat;

	XnStatus nRetVal = XN_STATUS_OK;
	NodeInfoList recordersList;
	nRetVal = context->EnumerateProductionTrees(XN_NODE_TYPE_RECORDER, NULL, recordersList);
	// take first
	NodeInfo chosen = *recordersList.Begin();

	pRecorder = new Recorder;
	nRetVal = context->CreateProductionTree(chosen, *pRecorder);

	nRetVal = pRecorder->SetDestination(XN_RECORD_MEDIUM_FILE, strFileName);

	return true;
}

//----------------------------------------
bool OpenNICapture::startCapture(){

	XnUInt64 nNow;
	xnOSGetTimeStamp(&nNow);
	nNow /= 1000;

	nStartOn = (XnUInt32)nNow;
	State = SHOULD_CAPTURE;

	return true;
}

//----------------------------------------
void OpenNICapture::stopCapture(){
	if (pRecorder != NULL){
		pRecorder->Release();
		delete pRecorder;
		pRecorder = NULL;
	}
}

//----------------------------------------
XnStatus OpenNICapture::captureFrame(){
	XnStatus nRetVal = XN_STATUS_OK;

	if (State == SHOULD_CAPTURE){
		XnUInt64 nNow;
		xnOSGetTimeStamp(&nNow);
		nNow /= 1000;

		// check if time has arrived
		if (nNow >= nStartOn)
		{
			// check if we need to discard first frame
			if (bSkipFirstFrame){
				bSkipFirstFrame = false;
			}else{
				// start recording
				for (int i = 0; i < CAPTURE_NODE_COUNT; ++i){
					nodes[i].nCapturedFrames = 0;
					nodes[i].bRecording = false;
				}
				State = CAPTURING;

				// add all captured nodes

                    if (device->m_Device.IsValid()){
                        nRetVal = pRecorder->AddNodeToRecording(device->m_Device, XN_CODEC_UNCOMPRESSED);
                        START_CAPTURE_CHECK_RC(nRetVal, "add device node");
                    }

                    if (device->m_DepthGenerator.IsValid() && nodes[CAPTURE_DEPTH_NODE].captureFormat!=XN_CODEC_NULL){
                        nRetVal = pRecorder->AddNodeToRecording(device->m_DepthGenerator, nodes[CAPTURE_DEPTH_NODE].captureFormat);
                        START_CAPTURE_CHECK_RC(nRetVal, "add depth node");
                        nodes[CAPTURE_DEPTH_NODE].bRecording = TRUE;
                        nodes[CAPTURE_DEPTH_NODE].pGenerator = &device->m_DepthGenerator;
                    }

                    if (device->m_ImageGenerator.IsValid() && nodes[CAPTURE_IMAGE_NODE].captureFormat!=XN_CODEC_NULL){
                        nRetVal = pRecorder->AddNodeToRecording(device->m_ImageGenerator, nodes[CAPTURE_IMAGE_NODE].captureFormat);
                        START_CAPTURE_CHECK_RC(nRetVal, "add image node");
                        nodes[CAPTURE_IMAGE_NODE].bRecording = TRUE;
                        nodes[CAPTURE_IMAGE_NODE].pGenerator = &device->m_ImageGenerator;
                    }

                    if (device->m_IrGenerator.IsValid() && nodes[CAPTURE_IR_NODE].captureFormat!=XN_CODEC_NULL){
                        nRetVal = pRecorder->AddNodeToRecording(device->m_IrGenerator, nodes[CAPTURE_IR_NODE].captureFormat);
                        START_CAPTURE_CHECK_RC(nRetVal, "add IR stream");
                        nodes[CAPTURE_IR_NODE].bRecording = TRUE;
                        nodes[CAPTURE_IR_NODE].pGenerator = &device->m_IrGenerator;
                    }
			}
		}
	}

	if (State == CAPTURING){
		// There isn't a real need to call Record() here, as the WaitXUpdateAll() call already makes sure
		// recording is performed.
		//nRetVal = pRecorder->Record();
		if (nRetVal != ((XnStatus)0)){
			return (nRetVal);
		}

		// count recorded frames
		for (int i = 0; i < CAPTURE_NODE_COUNT; ++i){
			if (nodes[i].bRecording && nodes[i].pGenerator->IsDataNew())
				nodes[i].nCapturedFrames++;
		}
	}
	return XN_STATUS_OK;
}

//----------------------------------------
void OpenNICapture::update(){
	captureFrame();
}
