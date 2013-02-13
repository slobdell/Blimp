
/*
#include "cv.h"
#include "highgui.h"
#include "Communicator.h"

#include "jpeglib.h"


#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "EDSDKErrors.h"
#include <iostream>
#include "CanonCameraWrapper.h"
#include "commonIncludes.h"



#include <Gdiplus.h>


#include <atlstr.h>
#include <atlimage.h>






DWORD WINAPI secondThread( LPVOID lpParam);
//DWORD WINAPI thirdThread( LPVOID lpParam);

CvCapture *capture = 0;
IplImage  *frame = 0;
IplImage  *grayFrame = 0;//new IplImage();
IplImage  *finalFrame = 0;
int       key = 0;
HANDLE mutex;
bool is_data_ready_for_copy;
bool is_data_ready_for_send;
Communicator *communicator;
std::vector<unsigned char> buf;
std::vector<int> p;
char size[4];
char *dataToSend;



IplImage* hBitmap2Ipl(HBITMAP hBmp, bool flip)
{
    BITMAP bmp;
    ::GetObject(hBmp,sizeof(BITMAP),&bmp);
 
    int    nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel/8;
    int    depth     = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;


	IplImage *img=cvCreateImageHeader(cvSize(bmp.bmWidth, bmp.bmHeight), depth, nChannels);
 
	img->imageData = (char*)malloc(bmp.bmHeight*bmp.bmWidth*nChannels*sizeof(char));
	memcpy(img->imageData,(char*)(bmp.bmBits),bmp.bmHeight*bmp.bmWidth*nChannels);

	IplImage *intermediateFrame=cvCreateImage(cvSize(160,120),IPL_DEPTH_8U,nChannels);
	cvResize(img, intermediateFrame, CV_INTER_LINEAR);

	IplImage *grayImage=cvCreateImage(cvGetSize(intermediateFrame), IPL_DEPTH_8U, 1);
	cvCvtColor(intermediateFrame, grayImage, CV_RGB2GRAY);

	if(flip)
		::cvFlip(grayImage, grayImage);


	return grayImage;
}



 


void main()

{
	
	//dataToSend=new char[1024*8];
	is_data_ready_for_copy=false;
	is_data_ready_for_send=false;
	
    
    capture = cvCaptureFromCAM( 0 );
 
    
	if ( !capture ) {}
 
    
    cvNamedWindow( "result", CV_WINDOW_AUTOSIZE );
	cvShowImage( "result", grayFrame );				

	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 160 );
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 120 );

	communicator=new Communicator();
	p.push_back(CV_IMWRITE_JPEG_QUALITY);
	p.push_back(7);

	mutex=CreateMutex(NULL, false, NULL);

	DWORD dwThreadId, dwThrdParam=1;
	HANDLE hThread1,hThread2,hThread3;
	hThread1=CreateThread(NULL, //no security attributes
						0,		//use default stack size
						secondThread,	//thread function
						NULL,	//argument to thread function
						0,				//use default creation flags
						&dwThreadId);
	
	

	CanonCameraWrapper *canonCameraWrapper=new CanonCameraWrapper();
	bool success=false;
	while(!success)
	{
		if(canonCameraWrapper->setup(0))success=true;
		else
		{
			std::cout<<"Camera not detected in system.  Retrying in 5 seconds\n";
			wait_ms(5000);
			//send a message that camera is not detected
		}

	}
	//send a message that the camera is now connected
	canonCameraWrapper->openSession();
	if(!canonCameraWrapper->beginLiveView())std::cout<<"Error here\n";
	wait_ms(4000);
	
	int dataSize;
	unsigned char *jpegData=NULL;
	EdsSize jpegSize;
	ATL::CImage image;

	CComPtr<IStream> stream;
	stream=NULL;
	while(1)
	{
		if(!(canonCameraWrapper->grabPixelsFromLiveView(0, dataSize, &jpegData, jpegSize)))
		{
			std::cout<<"got some kind of error\n";
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
			

			communicator->enqueueMessage("jpg\n",4);
			int n=buf.size();
			size[0]=(n>>24)&0xFF;
			size[1]=(n>>16)&0xFF;
			size[2]=(n>>8)&0xFF;
			size[3]=n&0xFF;
			communicator->enqueueMessage(size,4);
			communicator->enqueueMessage("\n",1);
			communicator->enqueueMessage(reinterpret_cast<char *>(&buf[0]), buf.size());
			communicator->enqueueMessage("\n",1);
			communicator->sendData();
			stream.Release();


		}
		
	}
	image.Destroy();

	if(false && image.GetBPP()!=24)
	{
		ATL::CImage reColoredImage;
		reColoredImage.Create(image.GetWidth(), image.GetHeight(), 24);//32 is bits per pixel...can be 1,4,8,etc
		image.StretchBlt(reColoredImage.GetDC(),0,0,image.GetWidth(), image.GetHeight(), SRCCOPY);
		reColoredImage.ReleaseDC();
		image=reColoredImage;
	}
	



	

	
	

	
	canonCameraWrapper->disableDownloadOnTrigger();
	canonCameraWrapper->focus();
	canonCameraWrapper->takePicture();
	
	
	canonCameraWrapper->endLiveView();
	canonCameraWrapper->closeSession();
	

	int counter=0;
    while( false && key != 'q')//  && index<10) {
	{
		counter++;

		DWORD dwWaitResult=WaitForSingleObject(mutex, INFINITE);
		if(dwWaitResult==WAIT_OBJECT_0)
		{
			frame = cvQueryFrame( capture );
			grayFrame=cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U,1);
			if( !frame ) break;
			
			cvCvtColor(frame, grayFrame, CV_BGR2GRAY);
			
			
			//convert to jpg

			//grayFrame is supposed to be a mat object here
			cv::imencode(".jpg", grayFrame,buf,p);
			
			if(counter%15==0)std::cout<<buf.size()<<"\n";
		       
			
			cvShowImage( "result", grayFrame );				
			is_data_ready_for_send=true;
			ReleaseMutex(mutex);
		}
		else
		{
			std::cout<<"mutex 2 fail\n";
			//mutex failed
		}
		
		

        key = cvWaitKey( 1 );
    }
	int junk;
	std::cin>>junk;

    
    cvDestroyWindow( "result" );
    cvReleaseCapture( &capture );
	
 
    
}

DWORD WINAPI secondThread( LPVOID lpParam)
{
	while(1)
	{
		DWORD dwWaitResult=WaitForSingleObject(mutex, INFINITE);
		if(dwWaitResult==WAIT_OBJECT_0)
		{
			//std::cout<<"send thread unlocked\n";
			if(is_data_ready_for_send)
			{
				//std::cout<<"SENDING\n";
				communicator->enqueueMessage("jpg\n",4);
				int n=buf.size();
				size[0]=(n>>24)&0xFF;
				size[1]=(n>>16)&0xFF;
				size[2]=(n>>8)&0xFF;
				size[3]=n&0xFF;
				communicator->enqueueMessage(size,4);
				communicator->enqueueMessage("\n",1);

				communicator->enqueueMessage(reinterpret_cast<char *>(&buf[0]), buf.size());
				communicator->enqueueMessage("\n",1);

				communicator->sendData();
				is_data_ready_for_send=false;
			}
			ReleaseMutex(mutex);
		}
		else
		{
			std::cout<<"mutex 2 fail\n";
			//mutex failed
		}
		
	}
	return 0;
}
/*

DWORD WINAPI thirdThread( LPVOID lpParam)
{
	while(1)
	{
		DWORD dwWaitResult=WaitForSingleObject(mutex, INFINITE);
		if(dwWaitResult==WAIT_OBJECT_0)
		{
			
			if(is_data_ready_for_copy && !is_data_ready_for_send)
			{
				dataSize=buf.size();
				memcpy(dataToSend, &(buf[0]), buf.size());
				is_data_ready_for_copy=false;
				is_data_ready_for_send=true;
	
			}
			ReleaseMutex(mutex);
		}
		else
		{
			std::cout<<"mutex 3 fail\n";
			//mutex failed
		}
		
	}
	return 0;
}
*/