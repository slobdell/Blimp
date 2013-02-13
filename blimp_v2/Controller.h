#pragma once
#ifndef Controller_H
#define Controller_H

#include "commonIncludes.h"
#include "GPSCoordinate.h"
#include "Altimeter_Sensor.h"
#include "Accelerometer_Sensor.h"
#include "Compass_Sensor.h"
#include "GPS_Sensor.h"
#include "PWMController.h"
#include "CameraManager.h"
#include <roboard.h>


#define ACCEPTABLE_ALTITUDE_ERROR_IN_FEET 10
#define FEET_ALTITUDE_PER_PITCH_ANGLE 5
#define TARGET_RADIUS_ACCURACY 15

class Controller
{
private:
	bool autoPilot, hover;
	int nothingYet;
	unsigned long ticker, ticker2;
	int savedHoverValue;
	int targetAltitude;
	void calculateHover();
	std::queue<GPSCoordinate *> gpsCoords;
	double currentLat, currentLong, targetLat, targetLong;
	double getDistanceInMeters(double lat1, double lon1, double lat2, double lon2);
	double getDistanceInMeters(GPSCoordinate *point1, GPSCoordinate *point2);
	int savedThrustVal;

public:
	Altimeter_Sensor *altimeter;
	Accelerometer_Sensor *accelerometer;
	Compass_Sensor *compass;
	GPS_Sensor *gps;
	PWMController *pwmController;
	Communicator *communicator;
	CameraManager *cameraManager;
	void sendGeneralMessage(std::string &inString);
	void setAutoPilot(bool trueOrFalse){autoPilot=trueOrFalse;}
	void setHover(bool trueOrFalse);
	bool isInAutoPilot(){return autoPilot;}
	
	void setTargetAltitude(int inVal){targetAltitude=inVal;}

	void think();
	void act();
	void readPassiveSensors();

	void stopEverything();

	void addGPSCoordinate(double inLat, double inLong);

	Controller();
	~Controller();
};

#endif