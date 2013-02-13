#include "GPS_Sensor.h"

GPS_Sensor::GPS_Sensor()
{
	comPort="COM9";//be default, but we can actually try and find the correct one
	currentLat=0;
	currentLong=0;
	com=new Tserial();
	findCorrectComPort();
	/*
	if(com!=0)
	{
		std::cout<<"Trying "<<comPort<<"\n";
		//com->disconnect();
		char *toSend=new char[comPort.size()];
		memcpy(toSend, comPort.c_str()+0, comPort.size()+1);
		std::cout<<toSend<<")))\n";
		
		com->connect(toSend, 4800, spNONE, false);
	}
	*/		
	
}
	
void GPS_Sensor::findCorrectComPort()
{
	int numTries=15;
	for(int tries=0;tries<numTries;tries++)
		for(int j=1;j<=16;j++)
		{
			comPort="COM"+to_string(j);
			if(com!=0)
			{
				
				com->disconnect();
				char *toSend=new char[5];
				memcpy(toSend, comPort.c_str()+0 /*terminates a C-style string*/, comPort.size()+1);
				com->connect(toSend, 4800, spNONE, false);
				wait_ms(100);
			}
			int bytesInBuffer=com->getNbrOfBytes();
			
			if(bytesInBuffer>0)
			{
				char *buf=new char[128];
				com->getArray(buf, 128);
				std::string output="";
				for( int j=0;j<128;j++)
				{
					output+=(char)buf[j];
				}
				//std::cout<<output<<"\n\n";
				if(output.find("$GPRMC") > 0 || output.find("$GPGGA")>0)
				{
					std::cout<<"GPS is using "<<comPort<<"\n";
					return;
				}
				else
				{
					std::cout<<output<<"\n\n";
				}
				
		
			}
			
			/*if(this->read()>0)
			{
				std::cout<<"got some data on "<<comPort<<"\n";
				if(this->getLat()!=0 && this->getLong()!=0)
				{
					//this is the correct port, yay!
					return;
				}
			}
			*/
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
	//SBL
	try{
	std::vector <std::string>lines=split(inVal,'\n');
	for(unsigned int j=0;j<lines.size();j++)
	{


		std::vector <std::string> data=split(lines[j],',');
		//std::cout<<data.size()<<"\n";
		//GPRMC
		
		if(data.size()>10 && data[0]=="$GPGGA" )//8 was arbitrary, I think it actually should be 6
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
	catch(int e)
	{
		std::cout<<"Error at cp 45\n";
	}
}

int GPS_Sensor::read()
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
	
	return bytesRead;
		
}
//SEE MY C# APPLICATION ON HOW TO IMPLEMENT THIS
GPS_Sensor::~GPS_Sensor()
{
	com->disconnect();
}