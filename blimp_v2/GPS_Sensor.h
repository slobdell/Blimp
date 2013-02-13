#pragma once
#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H
//#include <iostream>//including this multiple times now but for some reason this otherwise creates an error
#include "commonIncludes.h"


class GPS_Sensor : public Base_Sensor
{
private:
	std::string comPort;
	Tserial *com;
	char buffer[1024];
	std::string parseData(std::string &inVal);
	double currentLat, currentLong;
	void findCorrectComPort();
public:
	GPS_Sensor();
	int read();
	double getLat(){return currentLat;}
	double getLong(){return currentLong;}
	~GPS_Sensor();

};
#endif