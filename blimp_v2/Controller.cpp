#include "Controller.h"

Controller::Controller()
{
	
	communicator = new Communicator();
	
	std::string message="Powering up sensors and cameras...\n";
	communicator->sendGeneralMessage(message);
	altimeter=new Altimeter_Sensor();
	accelerometer=new Accelerometer_Sensor();
	compass=new Compass_Sensor();
	gps=new GPS_Sensor();

	
	pwmController = new PWMController();
	//cameraManager = new CameraManager(&communicator);
	altimeter->setStartAltitude();
	message="...done initializing\n";
	communicator->sendGeneralMessage(message);
	autoPilot=hover=false;
	savedHoverValue=-1;
	ticker, ticker2=0;
	targetAltitude=0;
	savedThrustVal=0;
	currentLat=currentLong=targetLat=targetLong=0;
	//gpsCoords=new Queue<GPSCoordinate *>();  guess I don't actually need this
	
}
void Controller::addGPSCoordinate(double inLat, double inLong)
{
	gpsCoords.push(new GPSCoordinate(inLat,inLong));
}
void Controller::stopEverything()
{
	pwmController->setThrustMagnitude(0);
	pwmController->goToNeutral();
}
void Controller::sendGeneralMessage(std::string &inString)
{
	communicator->sendGeneralMessage(inString);
}
void Controller::calculateHover()
{
	pwmController->setHover(true);//this is currently useless I think
	pwmController->setThrustVector(180);
	if(savedHoverValue!=-1)
	{
		pwmController->setThrustMagnitude(savedHoverValue);
		savedHoverValue=-1;
	}
	pwmController->start_yaw_left(false,0);
	pwmController->start_yaw_right(false,0);
	//first we're going to need to break until the velocity reaches 0
	

	//we could also go ahead and adjust the thrust angle as well

	int currentPitch=accelerometer->getPitch();
	if(abs(currentPitch)<ACCEPTABLE_PITCH_ERROR) //we're at 0 pitch, try and get pitch velocity as close to 0 as possible
		//ideal algorithm might be a binary search
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()>2)
		{
			pwmController->speedDown(2);
		}
		else if (accelerometer->getPitchVelocityDegreesPerSecond()<-2)
		{
			pwmController->speedUp(2);
		}
		else
		{

			//pitch velocity is pretty good, save this value for future use
			savedHoverValue=pwmController->getThrustMagnitudePercent();
			//pwmController->setThrustMagnitude
		}
	}
	else if(currentPitch>0 + ACCEPTABLE_PITCH_ERROR)
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()<0)
		{
			//we're already good
		}
		else if(accelerometer->getPitchVelocityDegreesPerSecond()>=0)
		{
			pwmController->speedDown(3);
		}
	}
	else if (currentPitch<ACCEPTABLE_PITCH_ERROR*-1)
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()>0)
		{
			//we're already good
		}
		else if(accelerometer->getPitchVelocityDegreesPerSecond()<=0)
		{
			pwmController->speedUp(3);
		}
	}
	
}
void Controller::setHover(bool trueOrFalse)
{
	bool previousVal=hover;
	hover=trueOrFalse;
	if(hover)
	{
		savedThrustVal=pwmController->getThrustMagnitudePercent();
	}
	if(previousVal==true && hover==false)
	{
		//slow down first
		pwmController->setThrustMagnitude(savedThrustVal);
		pwmController->goToNeutral();
	}

}

void Controller::think()
{


	if(hover)
	{
		calculateHover();
	}
	else
	{
		if(ticker%4!=0 || !autoPilot)return; //only thing every 4 ticks

		//alternate between reading the accelerometer and the compass
		if(ticker%2==0)accelerometer->read();
		else compass->read();
		ticker2++;

		/******************UPDATE TARGET LAT/LON********************************************/
		if(!(targetLat==0 && targetLong==0) && getDistanceInMeters(currentLat, currentLong, targetLat, targetLong)<	TARGET_RADIUS_ACCURACY && gpsCoords.size()>0)
		{
			gpsCoords.pop();
			if(gpsCoords.size()==0 && this->autoPilot)this->setHover(true);
		}
		if(gpsCoords.size()>0)
		{
			targetLat=gpsCoords.front()->lat;
			targetLong=gpsCoords.front()->lon;
		}
		else
		{
			targetLat=targetLong=0;
		}
		/******************END******************************************************************************/	
			
		
		/******************CALCULCATE TARGET PITCH FROM ALTITUDE********************************************/

		int currentAltitude=altimeter->getAltitude();
		int altitudeDistance=targetAltitude-currentAltitude;
		int targetPitch=altitudeDistance/FEET_ALTITUDE_PER_PITCH_ANGLE;  //1 degree for every y feet of altitude difference
		if(targetPitch>MAX_PITCH)targetPitch=MAX_PITCH;
		else if (targetPitch<MIN_PITCH)targetPitch=MIN_PITCH;
		
		pwmController->adjustPitch(accelerometer->getPitch(), accelerometer->getPitchVelocityDegreesPerSecond());
		/******************END******************************************************************************/


		/******************CALCULCATE TARGET AZIMUTH FROM COMPASS AND GPS********************************************/
		if(!(targetLat==0 && targetLong==0))
		{
			
			double deltaX=targetLong-currentLong;
			double deltaY=currentLat-targetLat;
			pwmController->setTargetAzimuth(atan2(deltaY, deltaX)*180/PI+90);
			
			pwmController->adjustYaw(compass->getAzimuth(), compass->getAzimuthVelocityDegreesPerSecond());
			//std::cout<<atan2(deltaY, deltaX)*180/PI+90<<"\n";
			//std::cout<<atan2(100.0,0.0)*180/PI<<"\n";//returns 90
		}
		else
		{
			//do nothing in terms of yaw
		}
		/******************END******************************************************************************/
	}
	

	


}

double Controller::getDistanceInMeters(GPSCoordinate *point1, GPSCoordinate *point2)
{
	return getDistanceInMeters(point1->lat, point1->lon, point2->lat, point2->lon);
	
}
double Controller::getDistanceInMeters(double lat1, double lon1, double lat2, double lon2)
{
	double d2r=PI/180.0;
	double dlong=(lon2-lon1)*d2r;
	double dlat=(lat2-lat1)*d2r;
	double a=pow(sin(dlat/2.0),2)+cos(lat1*d2r)*cos(lat2*d2r)*pow(sin(dlong/2.0),2);
	double c=2*atan2(sqrt(a), sqrt(1-a));
	double d=6367*c*1000;//convert to meters
	return d;
	
}
void Controller::act()
{
	ticker++;
	if(hover)return;
	pwmController->tick();

}
void Controller::readPassiveSensors()
{
	gps->read();
	
	if(!autoPilot)//if we're doing autopilot then this already being read in the think() function
	{
		accelerometer->read();
		wait_ms(100);
		compass->read();
	}

	//don't need to read these values because it's already being read elsewhere
	
	

	//I took out the locks here, maybe I need to add them back?
	communicator->enqueueMessage("lat\n",4);
	currentLat=gps->getLat();
	
	std::string lat = to_string(currentLat)+"\0";
	char* toSend=new char[lat.size()];
	memcpy(toSend, lat.c_str(), lat.size());
	//std::cout<<"Lat: "<<lat<<", "<<lat.size()<<"\n";
	//std::cout<<toSend<<"\n";
	communicator->enqueueMessage(toSend, lat.size());
	communicator->enqueueMessage("\n",1);
	

	communicator->enqueueMessage("lon\n",4);
	//delete toSend;
	currentLong=gps->getLong();
	std::string lon = to_string(currentLong)+"\0";
	toSend=new char[lon.size()];
	memcpy(toSend, lon.c_str(), lon.size());
	//std::cout<<toSend<<"\n";
	communicator->enqueueMessage(toSend, lon.size());
	communicator->enqueueMessage("\n",1);

	
	communicator->enqueueMessage("pit\n",4);
	std::string pitch=to_string(accelerometer->getPitch())+"\0";//we don't need to update this val because it's already being updated elsewhere
	//delete toSend;
	toSend=new char[pitch.size()];
	memcpy(toSend, pitch.c_str(), pitch.size());
	//std::cout<<toSend<<"\n";
	communicator->enqueueMessage(toSend, pitch.size());
	communicator->enqueueMessage("\n",1);


	communicator->enqueueMessage("azm\n", 4);
	int currentAz=compass->getAzimuth();
	std::string azimuth=to_string(currentAz)+"\0";//we don't need to update this val because it's already being updated elsewhere
	toSend=new char[azimuth.size()];
	memcpy(toSend, azimuth.c_str(), azimuth.size());
	//std::cout<<toSend<<"\n";
	communicator->enqueueMessage(toSend, azimuth.size());
	communicator->enqueueMessage("\n",1);	
	
	

	communicator->sendData();
		

	//it's also possible that the locks are causing this whole deal to F up
	
}
Controller::~Controller()
{
}