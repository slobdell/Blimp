#include "CameraManager.h"

CameraManager::CameraManager(Communicator **inCommunicator)
{
	communicator=*inCommunicator;
	capture = 0;
	frame = 0;
	grayFrame = 0;//new IplImage();
	finalFrame = 0;
	key = 0;
	currentCam=WEB_CAM;
	pauseVideoFeedForPicture=false;

	/*is_data_ready_for_copy=false;
	is_data_ready_for_send=false; 

	
	char size[4];
	char *dataToSend;*/
	canonCamInitialized=false;
	webCamInitialized=false;


	canonCameraWrapper=new CanonCameraWrapper();
	p.push_back(CV_IMWRITE_JPEG_QUALITY);
	p.push_back(7);
	this->initCameras();

}

IplImage* CameraManager::hBitmap2Ipl(HBITMAP hBmp, bool flip)
{
    BITMAP bmp;
    ::GetObject(hBmp,sizeof(BITMAP),&bmp);
 
    int    nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel/8;
    int    depth     = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;


	IplImage *img=cvCreateImageHeader(cvSize(bmp.bmWidth, bmp.bmHeight), depth, nChannels);
 
	img->imageData = (char*)malloc(bmp.bmHeight*bmp.bmWidth*nChannels*sizeof(char));
	memcpy(img->imageData,(char*)(bmp.bmBits),bmp.bmHeight*bmp.bmWidth*nChannels);

	IplImage *intermediateFrame=cvCreateImage(cvSize(TRANSMITTED_IMAGE_WIDTH,TRANSMITTED_IMAGE_HEIGHT),IPL_DEPTH_8U,nChannels);
	cvResize(img, intermediateFrame, CV_INTER_LINEAR);

	IplImage *grayImage=cvCreateImage(cvGetSize(intermediateFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(intermediateFrame, grayImage, CV_RGB2GRAY);

	if(flip)
		::cvFlip(grayImage, grayImage);


	return grayImage;
}

void CameraManager::sendData()
{
	int n=buf.size();
	size[0]=(n>>24)&0xFF;
	size[1]=(n>>16)&0xFF;
	size[2]=(n>>8)&0xFF;
	size[3]=n&0xFF;
	communicator->lock();
	communicator->enqueueMessage("jpg\n",4);
	communicator->enqueueMessage(size,4);
	communicator->enqueueMessage("\n",1);
	
	try{	 
	communicator->enqueueMessage(reinterpret_cast<char *>(&buf[0]), buf.size());
	}
	catch(int e)
	{
		std::cout<<"Out of range error sending camera video feed\n";
	}
	communicator->enqueueMessage("\n",1);
	communicator->unlock();
	communicator->sendData();
}
void CameraManager::initCameras()
{
	this->initCamera(WEB_CAM);
	this->initCamera(CANON_CAM);
}
void CameraManager::initCamera(int cameraNumber)
{
	
	if(cameraNumber==WEB_CAM)
	{
		if(webCamInitialized)return;
		std::cout<<"initializing camera\n";
		    /* initialize camera */
		capture = cvCaptureFromCAM( 0 );
		

		/* always check */
		if ( !capture ) {/*error */}
	 
		cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 160 );
		cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 120 );
		webCamInitialized=true;
	}
	else if(cameraNumber==CANON_CAM)
	{
		if(canonCamInitialized)return;
		std::cout<<"initializing camera\n";
		bool success=false;
		while(!success)
		{
			if(canonCameraWrapper->setup(0))success=true;
			else
			{
				std::cout<<"Camera not detected in system.  Retrying in 5 seconds\n";
				std::string message="Camera not detected in system.  Retrying in 5 seconds\n";
				communicator->sendGeneralMessage(message);
				wait_ms(5000);
				//send a message that camera is not detected
			}

		}
		//send a message that the camera is now connected
		canonCameraWrapper->openSession();
		canonCameraWrapper->disableDownloadOnTrigger();
		if(!canonCameraWrapper->beginLiveView())std::cout<<"Error here\n";
		wait_ms(4000);
		canonCamInitialized=true;
	}
	std::cout<<"Finished initializing cam\n";
}
void CameraManager::takePicture()
{
	if(currentCam==CANON_CAM)
	{
		pauseVideoFeedForPicture=true;
		canonCameraWrapper->focus();
		canonCameraWrapper->takePicture();
		generalMessage="...done\n";
		communicator->sendGeneralMessage(generalMessage);
	}
}
void CameraManager::closeCameras()
{
	this->closeCamera(WEB_CAM);
	this->closeCamera(CANON_CAM);
}
void CameraManager::closeCamera(int cameraNumber)
{
	std::cout<<"closing camera\n";
	if(cameraNumber==WEB_CAM)
	{
		//if(!webCamInitialized)return;
		webCamInitialized=false;
		cvReleaseCapture( &capture );
		
	}
	else if(cameraNumber==CANON_CAM)
	{

		//if(!canonCamInitialized)return;
		canonCameraWrapper->endLiveView();
		canonCameraWrapper->closeSession();
		canonCamInitialized=false;
	}
	std::cout<<"finished closing camera\n";
}
void CameraManager::tick()
{
	
	if(currentCam==WEB_CAM)
	{
		if(!webCamInitialized)return;
		frame = cvQueryFrame( capture );
		grayFrame=cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U,1);
		if( !frame ) return;
		cvCvtColor(frame, grayFrame, CV_BGR2GRAY);
	    finalFrame=cvCreateImage(cvSize(TRANSMITTED_IMAGE_WIDTH,TRANSMITTED_IMAGE_HEIGHT),IPL_DEPTH_8U,1);
		//cvResize(img, intermediateFrame, CV_INTER_LINEAR);
		cvResize(grayFrame, finalFrame, CV_INTER_LINEAR);
//cvNamedWindow( "result", CV_WINDOW_AUTOSIZE );
	//cvShowImage( "result", grayFrame );				
		cv::imencode(".jpg", finalFrame,buf,p);
	}
	else if(currentCam==CANON_CAM)
	{
		
		if(pauseVideoFeedForPicture)
		{
			//Sleep(4000);
			pauseVideoFeedForPicture=false;
			//MAKE SURE IMAGE REVIEW IS OFF ON THE CAMERA
			//return;
		}
		
		if(!canonCamInitialized)return;

		int dataSize;
		unsigned char *jpegData=NULL;
		EdsSize jpegSize;
		ATL::CImage image;

		CComPtr<IStream> stream;
		stream=NULL;
		
		if(!(canonCameraWrapper->grabPixelsFromLiveView(0, dataSize, &jpegData, jpegSize)))
		{
			std::string message="Camera error\n";
			communicator->sendGeneralMessage(message);
			//std::cout<<"got some kind of error\n";
		}
		else
		{
			HGLOBAL hMem = ::GlobalAlloc(GHND, dataSize); 
			LPVOID imageBuffer = ::GlobalLock(hMem);
			memcpy(imageBuffer, jpegData, dataSize);  //pBuff is now the image data
			::GlobalUnlock(hMem);
			CreateStreamOnHGlobal(hMem, TRUE, &stream);
			image.Load(stream);
			IplImage *pleaseWork=hBitmap2Ipl(image.Detach(), true);
			cv::imencode(".jpg", pleaseWork,buf,p);
			stream.Release();
		}
		
		image.Destroy();
	}
	this->sendData();
}
void CameraManager::changeCamera(int cameraNumber)
{
	//webCamInitialized=false;
	//canonCamInitialized=false;
	
	
	if(cameraNumber==WEB_CAM || cameraNumber==CANON_CAM)
	{
		currentCam=cameraNumber;
	}
	else
	{
		if(currentCam==WEB_CAM)currentCam=CANON_CAM;
		else currentCam=WEB_CAM;
	}
	
}
CameraManager::~CameraManager()
{
	this->closeCameras();
	
}