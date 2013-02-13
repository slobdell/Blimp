#include "Compass_Sensor.h"

Compass_Sensor::Compass_Sensor()
{
	i2c0_SetSpeed(I2CMODE_FAST, 400000L);
	
	i2c0master_StartN(i2c_address,I2C_WRITE,2);//write 2 byte
	i2c0master_WriteN(0x02); //mode register
	i2c0master_WriteN(0x00); //continue-measureture mode
	currentAzimuth=oldAzimuth=0;
	oldTicks=currentTicks=0;
	wait_ms(100);
}
void Compass_Sensor::read()
{
	oldAzimuth=currentAzimuth;
	oldTicks=currentTicks;
	currentTicks=GetTickCount();



	i2c0master_StartN(i2c_address, I2C_WRITE, 1);
	i2c0master_SetRestartN(I2C_READ, 6);
	i2c0master_WriteN(0x03); //Read from data register (Address : 0x03)
	d1 = i2c0master_ReadN();//X MSB
	d2 = i2c0master_ReadN();//X LSB
	d3 = i2c0master_ReadN();//Y MSB
	d4 = i2c0master_ReadN();//Y LSB
	d5 = i2c0master_ReadN();//Z MSB
	d6 = i2c0master_ReadN();//Z LSB
//std::cout<<d1<<", "<<d2<<", "<<d3<<", "<<d4<<", "<<d5<<", "<<d6<<"\n";
	x=((d1 & 0x80) != 0) ? (((~0)>>16)<<16) | ((d1<<8)+d2): (d1<<8)+d2;
    y=((d3 & 0x80) != 0) ? (((~0)>>16)<<16) | ((d3<<8)+d4): (d3<<8)+d4;
    z=((d5 & 0x80) != 0) ? (((~0)>>16)<<16) | ((d5<<8)+d6): (d5<<8)+d6;
	
	
		
		//std::cout<<x<<" "<<y<<":     ";//" "<<z<<"\n";
	result=atan((float)y/(float)x)*180.0/PI+90;
	
		//result=(180*(atan((double)(-1*(y/x))/PI)))+180;
	int quadrant;
	if(x>=0 && y>=0)quadrant=1;
	if(x<0 && y>=0)quadrant=2;
	if(x<0 && y<0)quadrant=3;
	if(x>=0 && y<0)quadrant=4;

	if(quadrant==4 || quadrant==1)result+=180;
	//don't need this if I'm using atan2

	/*if(quadrant==1)result=90-result;
	else result=450-result;
	result=(int)result%360;
	//bearing = (450-theta) mod 360*/
	//std::cout<<(int)result<<"\n";//<<quadrant<<"\n";
	currentAzimuth=result;
	if(x==0 && y==0 && z==0)currentAzimuth= -999;
}
int Compass_Sensor::getAzimuth()
{

	return currentAzimuth;
}
double Compass_Sensor::getAzimuthVelocityDegreesPerSecond()
{ 
	//if(currentTicks-oldTicks==0)return 0;
	double ret=(currentAzimuth-oldAzimuth)/((currentTicks-oldTicks)/1000.0); 
	
	if(abs((float)ret)>180)
	{
		ret=360+ret;
		while(ret-360>0)ret-=360;
		if(ret>180)ret=ret-360;
		//listening to Skrillex while coding this.   I just want to point that out.
	}
	return ret;
	
}//degreesPerSeconds
Compass_Sensor::~Compass_Sensor()
{
	i2c_Close();
}
