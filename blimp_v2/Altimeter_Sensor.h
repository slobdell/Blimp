#pragma once
#ifndef Altimeter_SENSOR_H
#define Altimeter_SENSOR_H

#include "commonIncludes.h"

const unsigned char LPS331AP_ADDR =	(0xba>>1); 
const unsigned char SHT21_ADDR	=		(0x80>>1);
const unsigned char BMP180_ADDR	=	(0xee>>1);




class Altimeter_Sensor : public Base_Sensor
{
private:
	unsigned char lps331ap_state, lps331ap_press[3], lps331ap_temp[2];
	int sht21_humi[2], sht21_temp[2];
	double lps331ap_mbar, lps331ap_deg, sht21_rh, sht21_deg, height;
	int count[4];
	int offset;
	int rawHeightFromSensor;
	void read();
public:
	Altimeter_Sensor();
	void setStartAltitude();
	int getAltitude();
	~Altimeter_Sensor();
};

#endif