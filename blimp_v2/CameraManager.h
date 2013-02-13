#pragma once
#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H
#include <iostream>
using  namespace std;

#include "cv.h"
#include "highgui.h"
#include "jpeglib.h"
#include "CanonCameraWrapper.h"

#include <atlstr.h>
#include <atlimage.h>
#include <Gdiplus.h>

#include "commonIncludes.h"




//#include "EDSDK.h"
//#include "EDSDKTypes.h"
//#include "EDSDKErrors.h"



#define WEB_CAM 0
#define CANON_CAM 1

#define TRANSMITTED_IMAGE_WIDTH 120
#define TRANSMITTED_IMAGE_HEIGHT 90

class CameraManager
{
private:
	int currentCam;
	CvCapture *capture;
	IplImage  *frame;
	IplImage  *grayFrame;//new IplImage();
	IplImage  *finalFrame;
	int       key;
	bool is_data_ready_for_copy;
	bool is_data_ready_for_send;
	Communicator *communicator;
	std::vector<unsigned char> buf;
	std::vector<int> p;
	char size[4];
	char *dataToSend;
	
	void sendData();
	bool canonCamInitialized, webCamInitialized;
	void initCameras();
	void closeCameras();
	void initCamera(int cameraNumber);
	void closeCamera(int cameraNumber);
	IplImage *hBitmap2Ipl(HBITMAP hBmp, bool flip);
	CanonCameraWrapper *canonCameraWrapper;
	bool pauseVideoFeedForPicture;
	std::string generalMessage;
public:
	CameraManager(Communicator **inCommunicator);
	void tick();
	void changeCamera(int cameraNumber);
	void takePicture();
	~CameraManager();

	

};

#endif