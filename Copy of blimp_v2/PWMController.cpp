/*

int channel=0;
unsigned long usedChannel;
unsigned long period=20000;
unsigned long duty=1000;
unsigned long count=300;
rcservo_EnterPWMMode();
rcservo_SendPWMPulses(channel, period, duty, count);
while(rcservo_IsPWMCompleted(channel)==false);

rcservo_Close();

*/

#include "PWMController.h"


PWMController::PWMController()
{
		hovering=false;
	    usedchannel = RCSERVO_USECHANNEL0 
		+RCSERVO_USECHANNEL1 
		+RCSERVO_USECHANNEL2 
		+RCSERVO_USECHANNEL3 
		+RCSERVO_USECHANNEL4 
		+RCSERVO_USECHANNEL5 
		+RCSERVO_USECHANNEL6 
		+RCSERVO_USECHANNEL7 
		+RCSERVO_USECHANNEL8 
		+RCSERVO_USECHANNEL9 
		+RCSERVO_USECHANNEL10 
		+RCSERVO_USECHANNEL11 
		+RCSERVO_USECHANNEL12 
		+RCSERVO_USECHANNEL13 
		+RCSERVO_USECHANNEL14 
		+RCSERVO_USECHANNEL15; // for RB-110 

		roboio_SetRBVer(RB_110); 
	//rcservo_Close(); 
	rcservo_Initialize(usedchannel);
	rcservo_EnterPWMMode(); 
	targetPitch=0;
	pitchingForward=false;
	pitchingBack=false;
	currentPitch=0;
	yawingLeft=false;
	yawingRight=false;

	braking=false;
	arm_motors();
	period  = (unsigned long)2000L;//GetPeriod(argv); 
	currentThrust=MIN_THRUST_VALUE;
	currentThrustPercent=0;
	currentAzimuth=targetAzimuth=yawIntensity=0;
	goToNeutral();
}

void PWMController::adjustYaw(int inCurrentYaw, double yawVelocity)
{

	currentAzimuth=inCurrentYaw;
	currentAzimuth=currentAzimuth%360;
	targetAzimuth=targetAzimuth%360;
	if(currentAzimuth>180)currentAzimuth=currentAzimuth-360;
	if(targetAzimuth>180)targetAzimuth=targetAzimuth-360;

	if(targetAzimuth-currentAzimuth<0+ACCEPTABLE_YAW_ERROR)
	{
		//yaw left
		start_yaw_left(true, 0);
	}
	else if(targetAzimuth-currentAzimuth>0+ACCEPTABLE_YAW_ERROR)
	{
		//yaw right
		start_yaw_right(true, 0);
	}

	currentAzimuth=inCurrentYaw;//I do this because for convention, I'll maintain 0-360 degrees in stead of 0...180 and 0....-180

}
void PWMController::adjustPitch(int inCurrentPitch, double pitchVelocity)
{
	currentPitch=inCurrentPitch;
	if(targetPitch-currentPitch<0+ACCEPTABLE_PITCH_ERROR)//we need to pitch up
	{
		if(pitchVelocity>5)//we're already in the process of pitching up
		{
			//don't do anything
			this->start_pitch_forward(false,0);
			this->start_pitch_backward(false,0);
		}
		else
		{
			this->start_pitch_backward(true,0);
		}
		//might need to implement something from pitching up too fast

	}
	else if(targetPitch-currentPitch > 0+ACCEPTABLE_PITCH_ERROR)//we need to pitch down
	{
		if(pitchVelocity<-5)
		{
			//don't do anything
			this->start_pitch_forward(false,0);
			this->start_pitch_backward(false,0);
		}
		else
		{
			this->start_pitch_forward(true,0);
		}
	}
	else //we're at the correct pitch
	{
		//we're at the correct pitch, do we need to make sure pitch velocity is 0??
		if(pitchVelocity>3)//pitch down
		{
			this->start_pitch_forward(true,0);
		}
		else if(pitchVelocity<-3)//pitch up
		{
			this->start_pitch_backward(true,0);
		}

	}
	if(currentPitch<MIN_PITCH)
	{
		start_pitch_backward(true, 0);
		//add support for bool var "iMightBeFalling"
	}
	else if(currentPitch>MAX_PITCH)start_pitch_forward(true, 0);
	

	//need access to the sensors here
	//
}
void PWMController::arm_motors()
{
	int *channels = new int [3];
	channels[0]=RIGHT_MOTOR;
	channels[1]=LEFT_MOTOR;
    unsigned long period  = (unsigned long)2000L;
    unsigned long duty    = (unsigned long)800L;
    unsigned long count   = 3500L;
	for(int j=0;j<2;j++)
	{
		std::cout<<"Arming channel "<<channels[j]<<"\n";
		int channel=channels[j];
		rcservo_SendPWM(channel, period, duty, count);
	}
	for(int j=0;j<2;j++)
	{
		int channel=channels[j];
		while(rcservo_IsPWMCompleted(channel)== false);
	}
	int j=2;
	channels[j]=RUDDER;
	int channel=channels[j];
	std::cout<<"Arming channel "<<channel<<"\n";
	duty=RUDDER_NEUTRAL_DUTY;//might need to add a value for here, this is the neutral val for last motor
	rcservo_SendPWM(channel, period, duty, count);
	
}
void PWMController::yaw_left()
{
	unsigned long leftThrust=currentThrust-50;
	unsigned long rightThrust=currentThrust+50;
	if(rightThrust > MAX_THRUST_VALUE)
	{
		unsigned long difference=rightThrust-MAX_THRUST_VALUE;
		leftThrust-=difference;
		rightThrust-=difference;
	}
	rcservo_SendPWM(LEFT_MOTOR, period, leftThrust, YAW_COUNT);
	rcservo_SendPWM(RIGHT_MOTOR, period, rightThrust, YAW_COUNT);

	while(rcservo_IsPWMCompleted(LEFT_MOTOR)== false);
	while(rcservo_IsPWMCompleted(RIGHT_MOTOR)== false);
	rcservo_SendCPWM(LEFT_MOTOR, period, currentThrust);
	rcservo_SendCPWM(RIGHT_MOTOR, period, currentThrust);
}
void PWMController::yaw_right()
{
	unsigned long leftThrust=currentThrust+50;
	unsigned long rightThrust=currentThrust-50;
	if(leftThrust>MAX_THRUST_VALUE)
	{
		unsigned long difference=leftThrust-MAX_THRUST_VALUE;
		leftThrust-=difference;
		rightThrust-=difference;
	}
	
	rcservo_SendPWM(LEFT_MOTOR, period, leftThrust, YAW_COUNT);
	rcservo_SendPWM(RIGHT_MOTOR, period, rightThrust, YAW_COUNT);

	while(rcservo_IsPWMCompleted(LEFT_MOTOR)== false);
	while(rcservo_IsPWMCompleted(RIGHT_MOTOR)== false);
	rcservo_SendCPWM(LEFT_MOTOR, period, currentThrust);
	rcservo_SendCPWM(RIGHT_MOTOR, period, currentThrust);
}



void PWMController::start_braking(bool startIfTrue_StopIfFalse)
{
	braking=startIfTrue_StopIfFalse;
	if(braking)
	{
		int temp=this->currentThrust;
		this->setThrustMagnitude(0);
		pitchingForward=pitchingBack=yawingLeft=yawingRight=false;
		currentPitchAngle=225;
		this->setThrustVector(currentPitchAngle);
		while(rcservo_IsPWMCompleted(THRUST_VECTOR)== false);
		this->setThrustMagnitude(currentThrust);
	}
	else if(!braking)
	{
		goToNeutral();
	}
}
void PWMController::goToNeutral()
{
	currentPitchAngle=135;
	this->setThrustVector(currentPitchAngle);

}
void PWMController::setThrustMagnitude(int percent_0_to_100)
{
	int range=MAX_THRUST_VALUE-MIN_THRUST_VALUE;
	currentThrustPercent=percent_0_to_100;
	unsigned long pwmVal=(percent_0_to_100*range)/100+MIN_THRUST_VALUE;
	currentThrust=pwmVal;
	
	duty=currentThrust;
	//std::cout<<"setting motors to "<<currentThrust<<"\n";
	rcservo_SendCPWM(LEFT_MOTOR, period, duty);
	rcservo_SendCPWM(RIGHT_MOTOR, period, duty);
		
}
void PWMController::setThrustVector(int angle_0_to_360)
{
	angle_0_to_360=angle_0_to_360%360;
	unsigned long count=1;
	int min=300;
	int max=1800;
	int range=max-min;
	//duty=angle_0_to_360*range/360+min;
	duty=max-(angle_0_to_360*range/360);
	rcservo_SendPWM(THRUST_VECTOR, period, duty, count);
	
	//rcservo_SendCPWM(THRUST_VECTOR, period, duty);
}
void PWMController::pitch_forward()
{
	currentPitchAngle-=5;
	this->setThrustVector(currentPitchAngle);

}
void PWMController::pitch_back()
{
	currentPitchAngle+=5;
	this->setThrustVector(currentPitchAngle);
	
}
void PWMController::genericTest(int servoNumber, int inDuty)
{
	rcservo_SendPWM(servoNumber, period, inDuty, 30);
}
void PWMController::genericTest(int servoNumber, int inDuty, int inCount)
{
	rcservo_SendPWM(servoNumber, period, inDuty, inCount);
}
void PWMController::start_yaw_left(bool startIfTrue_StopIfFalse, int intensity)
{
	this->yawingLeft=startIfTrue_StopIfFalse;
	if(yawingLeft && yawingRight)
	{
		yawingRight=false;
	}
}
void PWMController::start_yaw_right(bool startIfTrue_StopIfFalse, int intensity)
{
	this->yawingRight=startIfTrue_StopIfFalse;
	if(yawingLeft && yawingRight)
	{
		yawingLeft=false;
	}
}

void PWMController::start_pitch_forward(bool startIfTrue_StopIfFalse, int intensity)
{
	pitchingForward=startIfTrue_StopIfFalse;
	if(pitchingForward && pitchingBack)
	{
		pitchingBack=false;
	}
	

}
void PWMController::start_pitch_backward(bool startIfTrue_StopIfFalse, int intensity)
{
	pitchingBack=startIfTrue_StopIfFalse;
	if(pitchingForward && pitchingBack)
	{
		pitchingForward=false;
	}

	
}
//this function does no calculations, it just acts on the existing calculations already made
void PWMController::tick()
{
	
	bool moved=false;
	if(braking)
	{
		if(pitchingForward)
		{
			if(currentPitchAngle-ANGLE_STEP_VAL>=180)
			{
				currentPitchAngle+=ANGLE_STEP_VAL;
				moved=true;
				
			}
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 800);
		}
		else if (pitchingBack)
		{
			if(currentPitchAngle+ANGLE_STEP_VAL<=270)
			{
				currentPitchAngle-=ANGLE_STEP_VAL;
				moved=true;
				
			}
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 1990);
		}
		else
		{
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 1600);
			//might want to add a line to set it to current angle for a few counts
		}
	}
	else if(!braking)
	{
		if(pitchingForward)
		{
			if(currentPitchAngle-ANGLE_STEP_VAL>=0)
			{
				currentPitchAngle-=ANGLE_STEP_VAL;
				moved=true;
				
			}
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 800);
		}
		else if (pitchingBack)
		{
			if(currentPitchAngle+ANGLE_STEP_VAL<=180)
			{
				currentPitchAngle+=ANGLE_STEP_VAL;
				moved=true;
				
			}
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 1990);
		}

		else
		{
			::rcservo_StopPWM(ELEV);
			rcservo_SendCPWM(ELEV, period, 1600);
			//might want to add a line to set it to current angle for a few counts
		}
	}

	int midPoint=(YAW_RIGHT_DUTY_MAX+YAW_LEFT_DUTY_MAX)/2;
	if(yawingLeft || yawingRight)
	{
		yawIntensity+=YAW_INTENSITY_STEP_VAL;
		if(yawIntensity>100)yawIntensity=100;

	}
	if(yawingLeft)
	{
		::rcservo_StopPWM(RUDDER);
		//rcservo_SendCPWM(RUDDER, period, 1990);
		int yawDuty=(YAW_LEFT_DUTY_MAX-midPoint)*yawIntensity+midPoint;
		::rcservo_SendPWM(RUDDER, period, yawDuty, 3);

		

	}
	else if(yawingRight)
	{
		::rcservo_StopPWM(RUDDER);
		//rcservo_SendCPWM(RUDDER, period, 800);
		int yawDuty=(YAW_RIGHT_DUTY_MAX-midPoint)*yawIntensity+midPoint;
		::rcservo_SendPWM(RUDDER, period, yawDuty, 3);
	}
	else
	{
		::rcservo_StopPWM(RUDDER);
		rcservo_SendCPWM(RUDDER, period, RUDDER_NEUTRAL_DUTY);
	}
	if(braking)
	{
		//here we'll need to adjust the currentPitchAngle based on the angle of the blimp
	}
	this->setThrustVector(currentPitchAngle);



	
	


		
}

void PWMController::speedUp(int percent_0_to_100)
{
	currentThrustPercent+=percent_0_to_100;
	if(currentThrustPercent<0)currentThrustPercent=0;
	this->setThrustMagnitude(currentThrustPercent);

}
void PWMController::speedDown(int percent_0_to_100)
{
	currentThrustPercent-=percent_0_to_100;
	if(currentThrustPercent>100)currentThrustPercent=100;
	this->setThrustMagnitude(currentThrustPercent);
}

PWMController::~PWMController()
{
	rcservo_Close();
}

