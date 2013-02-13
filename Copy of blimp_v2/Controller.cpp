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
	cameraManager = new CameraManager(&communicator);
	altimeter->setStartAltitude();
	message="...done initializing\n";
	communicator->sendGeneralMessage(message);
	autoPilot=hover=false;
	savedHoverValue=-1;
	ticker=0;
	targetAltitude=0;
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
	//first we're going to need to break until the velocity reaches 0
	

	//we could also go ahead and adjust the thrust angle as well

	int currentPitch=accelerometer->getPitch();
	if(currentPitch>ACCEPTABLE_PITCH_ERROR)
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()<0)
		{
			//we're already good
		}
		else if(accelerometer->getPitchVelocityDegreesPerSecond()>0)
		{
			pwmController->speedDown(5);
		}
	}
	else if (currentPitch<ACCEPTABLE_PITCH_ERROR*-1)
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()>0)
		{
			//we're already good
		}
		else if(accelerometer->getPitchVelocityDegreesPerSecond()<0)
		{
			pwmController->speedUp(5);
		}
	}
	else //we're at 0 pitch, try and get pitch velocity as close to 0 as possible
		//ideal algorithm might be a binary search
	{
		if(accelerometer->getPitchVelocityDegreesPerSecond()>2)
		{
			pwmController->speedDown(1);
		}
		else if (accelerometer->getPitchVelocityDegreesPerSecond()<-2)
		{
			pwmController->speedUp(1);
		}
		else
		{

			//pitch velocity is pretty good, save this value for future use
			savedHoverValue=pwmController->getThrustMagnitudePercent();
			//pwmController->setThrustMagnitude
		}
	}
}
void Controller::setHover(bool trueOrFalse)
{
	bool previousVal=hover;
	hover=trueOrFalse;
	if(previousVal==true && hover==false)
	{
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

		accelerometer->read();
		pwmController->adjustPitch(accelerometer->getPitch(), accelerometer->getPitchVelocityDegreesPerSecond());

		compass->read();
		pwmController->adjustYaw(compass->getAzimuth(), compass->getAzimuthVelocityDegreesPerSecond());

		int currentAltitude=altimeter->getAltitude();

		if(targetAltitude-currentAltitude>0+ACCEPTABLE_ALTITUDE_ERROR_IN_FEET)
		{
			pwmController->setTargetPitch(30);
		}
		else if(targetAltitude-currentAltitude<0-ACCEPTABLE_ALTITUDE_ERROR_IN_FEET)
		{
			pwmController->setTargetPitch(-20);
		}
		else
		{
			pwmController->setTargetPitch(0);
		}
		
		//ALTIMETER AND STUFF TAKES 100 ms to read!!

	}
	

	


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
	accelerometer->read();

	//I took out the locks here, maybe I need to add them back?
	std::string lat = to_string(gps->getLat())+"\n";
	std::string needToPassAString="lat\n";
	communicator->sendGeneralMessage(needToPassAString);
	communicator->sendGeneralMessage(lat);
	needToPassAString="lon\n";
	std::string lon = to_string(gps->getLong())+"\n";
	communicator->sendGeneralMessage(needToPassAString);
	communicator->sendGeneralMessage(lon);
	//it's also possible that the locks are causing this whole deal to F up
	
}
Controller::~Controller()
{
}