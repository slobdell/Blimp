#include "Accelerometer_Sensor.h"

Accelerometer_Sensor::Accelerometer_Sensor()
{
	i2c0_SetSpeed(I2CMODE_FAST, 400000L);
	i2c0master_StartN(acceler_addr, I2C_WRITE, 2);//write 2 bytes
	i2c0master_WriteN(0x2d);//power control register
	i2c0master_WriteN(0x28);//link and measure mode

	i2c0master_StartN(acceler_addr, I2C_WRITE, 2);//write 2 bytes
	i2c0master_WriteN(0x31);//data format register
	i2c0master_WriteN(0x08);//full resolution

	i2c0master_StartN(acceler_addr, I2C_WRITE, 2);//write 2 bytes
	i2c0master_WriteN(0x38);//FIFO Control register
	i2c0master_WriteN(0x00);//bypass mode
	oldPitch=currentPitch=currentTicks=oldTicks=0;
	wait_ms(100);

}

void Accelerometer_Sensor::read()
{
	i2c0master_StartN(acceler_addr, I2C_WRITE, 1);
	i2c0master_SetRestartN(I2C_READ, 6);
	i2c0master_WriteN(0x32);//read from X register address 0x32
	d1=i2c0master_ReadN();//X LSB
	d2=i2c0master_ReadN();//X MSB
	d3=i2c0master_ReadN();//Y LSB
	d4=i2c0master_ReadN();//Y MSB
	d5=i2c0master_ReadN();//Z LSB
	d6=i2c0master_ReadN();//Z MSB
	xAcceleration=((d2&0x80) !=0)?(((~0)>>16)<<16) | ((d2<<8)+d1):(d2<<8)+d1;
	yAcceleration=((d4&0x80) !=0)?(((~0)>>16)<<16) | ((d4<<8)+d3):(d4<<8)+d3;
	zAcceleration=((d6&0x80) !=0)?(((~0)>>16)<<16) | ((d6<<8)+d5):(d6<<8)+d5;
	
	
	//std::cout<<"x: "<<xAcceleration<<", "<<"y: "<<yAcceleration<<", "<<"z: "<<zAcceleration<<"\n";
	//atan2(accelerometer/gravity, z/gravity);
	//atan2(y/z);
	//std::cout<<sqrt((float)(xAcceleration*xAcceleration+yAcceleration*yAcceleration+zAcceleration*zAcceleration))<<"\n";

	
	//I dunno wtf  std::cout<<"Roll??: "<<180*atan2(zAcceleration,sqrt((float)(yAcceleration*yAcceleration+xAcceleration*xAcceleration)))/PI<<"\n";
	//std::cout<<"\n";

		//if x is positive, roll left
		//if x is negative, roll right
		//if y is positive, pitch up
		//if y is negative, pitch down
		//if z is negative, we're upside down
		//if z is positive, we're right side up
}
int Accelerometer_Sensor::getPitch()
{
	oldPitch=currentPitch;
	oldTicks=currentTicks;
	currentTicks=GetTickCount();

	

	currentPitch=180*atan2(yAcceleration,sqrt((float)(xAcceleration*xAcceleration+zAcceleration*zAcceleration)))/PI;
	return currentPitch;

	//pitch: atan(x/sqrt(y^2+z^2))
	//roll atan(y/sqrt(z^2 z^2)) <--is this supposed to be an x???
	

}
int Accelerometer_Sensor::getRoll()
{
	return 180*atan2(xAcceleration,sqrt((float)(yAcceleration*yAcceleration+zAcceleration*zAcceleration)))/PI;
}

Accelerometer_Sensor::~Accelerometer_Sensor()
{
	i2c_Close();
}