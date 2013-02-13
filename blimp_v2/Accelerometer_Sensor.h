#pragma once
#ifndef Accelerometer_SENSOR_H
#define Accelerometer_SENSOR_H

#include "commonIncludes.h"

const unsigned char acceler_addr=0x53;

class Accelerometer_Sensor : public Base_Sensor
{
private:
	int xAcceleration, yAcceleration, zAcceleration;
	double oldPitch, currentPitch;
	unsigned int d1,d2,d3,d4,d5,d6;
	unsigned long oldTicks, currentTicks;
public:
	Accelerometer_Sensor();
	void read();
	int getPitch();
	int getRoll();
	double getPitchVelocityDegreesPerSecond(){ return (currentPitch-oldPitch)/((currentTicks-oldTicks)/1000.0); }//degreesPerSeconds
	~Accelerometer_Sensor();
};

#endif