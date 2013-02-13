#pragma once
#ifndef COMPASS_SENSOR_H
#define COMPASS_SENSOR_H

#include "commonIncludes.h"
const unsigned char i2c_address = 0x1e;

class Compass_Sensor : public Base_Sensor
{

private:
	
	unsigned int d1,d2,d3,d4,d5,d6;
	int x,y,z;
	double result;
	int currentAzimuth, oldAzimuth;
	unsigned long oldTicks, currentTicks;

public:
	Compass_Sensor();
	void read();
	int getAzimuth();
	double getAzimuthVelocityDegreesPerSecond();
	
	~Compass_Sensor();

};
#endif