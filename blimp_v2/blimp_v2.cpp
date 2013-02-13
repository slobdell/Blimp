// blimp_v2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "commonIncludes.h"


#include "Controller.h"



unsigned char altimeter_addr=0xec;//EE, then EC


DWORD WINAPI CommunicatorListener( LPVOID lpParam);
DWORD WINAPI CommunicatorSender( LPVOID lpParam);
DWORD WINAPI SensorReader( LPVOID lpParam);
DWORD WINAPI VideoFeed( LPVOID lpParam);
DWORD WINAPI FlightController( LPVOID lpParam);
std::string generalMessage;

int _tmain(int argc, _TCHAR* argv[])
{
	
	roboio_SetRBVer(RB_110); // use RB-100
	//useless i2c_Close();//flush out bad stuff I think
    if (i2c_Initialize(I2CIRQ_DISABLE) == false)
	{
		printf("FALSE!!  %s\n", roboio_GetErrMsg());
		
	}
	i2c0_SetSpeed(I2CMODE_FAST, 400000L);


	
	Controller *mainController=new Controller();
	
	while(!mainController->communicator->establishCommunications())
	{
		std::cout<<"Failed to establish communications.  Retrying in 5 seconds\n";
		Sleep(5000);
	}
	
	
	//std::cout<<mainController->compass->getAzimuth()<<"\n";
	//std::cout<<mainController->compass->getAzimuthVelocityDegreesPerSecond()<<"\n\n";
		
	
	/*
		while(1)
	{
		mainController->compass->read();
		int currentAz=mainController->compass->getAzimuth();
		
		if(currentAz!=-999)std::cout<<"Az: "<<mainController->compass->getAzimuth()<<"\n";

		mainController->accelerometer->read();
		int currentP=mainController->accelerometer->getPitch();
		if(currentP!=-999)std::cout<<"Pitch: "<<currentP<<"\n";
			
		
		Sleep(100);
	}*/
	
	int counter=0;
	DWORD dwThreadId, dwThrdParam=1;
	HANDLE hThread1,hThread2,hThread3;
	hThread1=CreateThread(NULL, //no security attributes
						0,		//use default stack size
						CommunicatorListener,	//thread function
						mainController,	//argument to thread function
						0,				//use default creation flags
						&dwThreadId);
	
	hThread2=CreateThread(NULL, //no security attributes
						0,		//use default stack size
						CommunicatorSender,	//thread function
						mainController,	//argument to thread function
						0,				//use default creation flags
						&dwThreadId);
	hThread3=CreateThread(NULL, 0,SensorReader,	mainController,	0, &dwThreadId);
	HANDLE hThread4=CreateThread(NULL, 0,VideoFeed,	mainController,	0, &dwThreadId);
	HANDLE hThread5=CreateThread(NULL, 0,FlightController,	mainController,	0, &dwThreadId);


	HANDLE currentThread=GetCurrentThread();
	::SetThreadPriority(currentThread, THREAD_PRIORITY_IDLE);
	::SetThreadPriority(hThread1, THREAD_PRIORITY_BELOW_NORMAL);
	::SetThreadPriority(hThread2, THREAD_PRIORITY_BELOW_NORMAL);
	::SetThreadPriority(hThread3, THREAD_PRIORITY_BELOW_NORMAL);
	
	::SetThreadPriority(hThread4, THREAD_PRIORITY_BELOW_NORMAL);
	::SetThreadPriority(hThread5, THREAD_PRIORITY_ABOVE_NORMAL);
	//mainController->setHover(true);

	//mainController->setAutoPilot(true);
	//mainController->pwmController->setTargetAzimuth(100);

	while(1)
	{
		//mainController->communicator->listen();
		
		
		Sleep(0);
		//wait_ms(5000);
	}
	
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	CloseHandle(hThread3);
	CloseHandle(hThread4);
	std::cout<<"End of main\n";
	int junk;
	std::cin>>junk;
	return 0;

}


DWORD WINAPI CommunicatorListener( LPVOID lpParam)
{
	std::cout<<"Communicator Listener\n";
	Controller *controller=static_cast<Controller*>(lpParam);
	std::string oldData="";
	while(1)
	{
		try{
		std::string data=controller->communicator->listen();
		
		if(data!="")
		{
			if(oldData!="")
			{
				data=oldData+"\n"+data;//this can cause a problem because you're actually injecting a newline that wasn't necessarily there
				//std::cout<<data;
				oldData="";
			}
			std::vector <std::string>lines=split(data,'\n');
			
			while(lines.size()>0)
			{
				try{
					std::string myjunk=lines[0];
				}
				catch(int e)
				{
					std::cout<<"SBL AAAAAJJJJAAAA Error at location 0!!!!!!!!\n";
				}
			//	std::cout<<lines[0];
				if(lines[0]=="pch")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="f")
					{
						//controller->pwmController->pitch_forward();
						controller->pwmController->start_pitch_forward(true,0);
					}
					else if(lines[1]=="fs")
					{
						//controller->pwmController->pitch_forward();
						controller->pwmController->start_pitch_forward(false,0);
					}
					else if(lines[1]=="b")
					{
						controller->pwmController->start_pitch_backward(true,0);
					}
					else if(lines[1]=="bs")
					{
						controller->pwmController->start_pitch_backward(false,0);
					}
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="xbee")
				{
					//ignore it
					lines.erase(lines.begin());
				}
				else if(lines[0]=="yaw")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="l")
					{
						controller->pwmController->start_yaw_left(true, 0);
					}
					else if(lines[1]=="ls")
					{
						controller->pwmController->start_yaw_left(false, 0);
					}
					else if(lines[1]=="r")
					{
						controller->pwmController->start_yaw_right(true, 0);
					}
					else if(lines[1]=="rs")
					{
						controller->pwmController->start_yaw_right(false, 0);
					}
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="spd")
				{
					if(lines.size()<2)
					{
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					int value=atoi(lines[1].c_str());
					value*=10;//current conversion to convert to a percent
					
					controller->pwmController->setThrustMagnitude(value);
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="alt")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					std::cout<<data<<"\n";
					int value=atoi(lines[1].c_str());
					controller->setTargetAltitude(value);
					std::cout<<"Setting altitude to "<<value<<"\n";
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="cam")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="pic")
					{
						generalMessage="Taking a pic...\n";
						controller->sendGeneralMessage(generalMessage);
						controller->cameraManager->takePicture();
						
					}
					else
					{
						generalMessage="Changing cam...\n";
						std::cout<<"changing camera\n";
						controller->sendGeneralMessage(generalMessage);

						int value=atoi(lines[1].c_str());
						controller->cameraManager->changeCamera(value);
						
					}
					lines.erase(lines.begin(), lines.begin()+2);

					
				}
				else if(lines[0]=="arm")
				{
					controller->pwmController->arm_motors();
					lines.erase(lines.begin());
				}
				else if(lines[0]=="hov")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="b")
					{
						//controller->setAutoPilot(true);
						//start hovering
						controller->setHover(true);
						
						
					}					
					else if(lines[1]=="s")
					{
						//stop hovering
						controller->setHover(false);
						
					}
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="neu")
				{
					controller->pwmController->goToNeutral();
					lines.erase(lines.begin());
				}
				else if(lines[0]=="brk")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="b")
					{
						controller->pwmController->start_braking(true);
						
					}					
					else if(lines[1]=="s")
					{
						controller->pwmController->start_braking(false);
						
					}
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else if(lines[0]=="gps")
				{
					if(lines.size()<3)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						if(lines.size()==2)oldData=oldData+"\n"+lines[1];
						std::cout<<"breaking GPS\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					double lat=atof(lines[1].c_str());
					double lon=atof(lines[2].c_str());
					controller->addGPSCoordinate(lat, lon);
					lines.erase(lines.begin(), lines.begin()+3);
				}
				else if(lines[0]=="aut")
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					else if(lines[1]=="b")
					{
						controller->setAutoPilot(true);
						
					}					
					else if(lines[1]=="s")
					{
						controller->setAutoPilot(false);
						
					}
					lines.erase(lines.begin(), lines.begin()+2);
				}
				else
				{
					if(lines.size()<2)
					{
						if(lines.size()==0)std::cout<<"SBL THIS IS THE ERROR\n";
						oldData=lines[0];
						std::cout<<"breaking\n";
						std::cout<<oldData<<" SBL\n";
						break;
					}
					//std::cout<<"Piss: "<<lines[0]<<"\n";
					lines.erase(lines.begin());
				}
			}
			
		}
		//wait_ms(500);
		::Sleep(100);
		}
		catch(int e)
		{
			std::cout<<"SBL found it in the listener thread\n";
		}
	}
	
	return 0;
}
DWORD WINAPI CommunicatorSender( LPVOID lpParam)
{
	
	std::cout<<"Communicator Sender\n";
	wait_ms(250);
	Controller *controller=static_cast<Controller*>(lpParam);
	while(1)
	{
		controller->communicator->sendData();
		//wait_ms(500);
		Sleep(180);
		
	}
	return 0;
}
DWORD WINAPI SensorReader( LPVOID lpParam)
{
	
	std::cout<<"Sensor Reader\n";
	Controller *controller=static_cast<Controller*>(lpParam);
	while(1)
	{
		controller->readPassiveSensors();
		Sleep(500);//SBL
	}
	return 0;
}

DWORD WINAPI VideoFeed( LPVOID lpParam)
{
	
	std::cout<<"Video Feed\n";
	return 0;
	Controller *controller=static_cast<Controller*>(lpParam);
	while(1)
	{
		
		controller->cameraManager->tick();
		Sleep(250);
	}
	return 0;
}
DWORD WINAPI FlightController( LPVOID lpParam)
{
	
	std::cout<<"Flight Controller\n";
	Controller *controller=static_cast<Controller*>(lpParam);
	controller->pwmController->arm_motors();
	while(1)
	{
		controller->think();
		controller->act();
		//controller->pwmController->tick();
//adjust the target pitch based on the altitude
		//wait_ms(150);
		Sleep(30);
	}
}