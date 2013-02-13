#include "GPS_Sensor.h"

GPS_Sensor::GPS_Sensor()
{
	currentLat=0;
	currentLong=0;
	com=new Tserial();
	if(com!=0)
	{
		com->connect("COM8", 4800, spNONE, false);
		//need to add feature to determine proper port
		
	}
}
double someConversion(double inVal)
{
	int degrees=(int)(inVal/100.0);
	double decimalDegrees=(inVal-(100*degrees))/60.0;
	double fullVal=degrees+decimalDegrees;
	return fullVal;
}

std::string GPS_Sensor::parseData(std::string &inVal)
{
	
	std::vector <std::string>lines=split(inVal,'\n');
	for(unsigned int j=0;j<lines.size();j++)
	{


		std::vector <std::string> data=split(lines[j],',');
		//std::cout<<data.size()<<"\n";
		if(data[0]=="$GPGGA" && data.size()>10)//8 was arbitrary, I think it actually should be 6
		{
			
			char *NESW=new char[2];
			double dlat=atof(data[2].c_str());
			double fullLat=someConversion(dlat);
			NESW[0]=data[3][0];
			if(NESW[0]=='S')
				fullLat*=-1;
			double dlong=atof(data[4].c_str());
			double fullLong=someConversion(dlong);
			NESW[1]=data[5][0];
			if(NESW[1]=='W')
				fullLong*=-1;
			//std::cout<<fullLat<<", "<<fullLong<<"\n";
			currentLat=fullLat;
			currentLong=fullLong;


		}

	}
	return "";
}

void GPS_Sensor::read()
{
	//std::cout<<"Reading GPS Data\n";
	int bytesRead=com->getArray(buffer, 1024);
	//printf("Number of bytes = %d\n", bytesRead);
	std::string output="";
	for( int j=0;j<bytesRead;j++)
	{
		//printf("%c",buffer[j]);
		output+=(char)buffer[j];
	}
	std::string final=parseData(output);
		
		
}
//SEE MY C# APPLICATION ON HOW TO IMPLEMENT THIS
GPS_Sensor::~GPS_Sensor()
{
	com->disconnect();
}