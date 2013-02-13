#pragma once
#ifndef BASE_SENSOR_H
#define BASE_SENSOR_H
#include "tserial.h"

class Base_Sensor
{
private: 
	Tserial *com;
public:
	Base_Sensor(){}
};

#endif
/*
 Tserial *com;
    char buffer[1024];
	int i;
    com = new Tserial();
    if (com!=0)
    {
        com->connect("COM5", 4800, spNONE);
        int bytesRead=com->getArray(buffer, 1024);
		printf("Number of bytes = %d\n",bytesRead);
        for (i=0; i<bytesRead; i++)
        {
            printf("%c",buffer[i]);
        }
        printf("_\n");
        printf("Number of bytes = %d\n",com->getNbrOfBytes());

        com->sendArray("Hello World !",11);


        // ------------------
        com->disconnect();

        // ------------------
        delete com;
        com = 0;
    }
	int junk;
	cin>>junk;
    return 0;
*/