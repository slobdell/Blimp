#pragma once
#ifndef GPSCOORDINATE_H
#define GPSCOORDINATE_H



class GPSCoordinate
{
public:
	GPSCoordinate(double inLat, double inLon);
	double lat, lon;
};

#endif