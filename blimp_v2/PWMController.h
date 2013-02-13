#pragma once
#ifndef PWMCONTROLLER_H
#define PWMCONTROLLER_H

#include "commonIncludes.h"

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1
#define ELEV 2
#define RUDDER 3
#define GEAR 4
#define THRUST_VECTOR 5

#define MIN_THRUST_VALUE 800
#define MAX_THRUST_VALUE 1700
#define YAW_COUNT 100
#define ANGLE_STEP_VAL 1
#define YAW_INTENSITY_STEP_VAL 1
#define ELEV_PERCENT_STEP_VAL 1
#define NEUTRAL_ANGLE 135
#define VERTICAL_ANGLE 180

#define RUDDER_NEUTRAL_DUTY 1600
#define YAW_RIGHT_DUTY_MAX 800
#define YAW_LEFT_DUTY_MAX 1990

#define ACCEPTABLE_PITCH_ERROR 2
#define ACCEPTABLE_YAW_ERROR 3

#define MAX_PITCH 40
#define MIN_PITCH -30



#define ELEV_DUTY_NEUTRAL 1600
#define ELEV_DUTY_PITCH_UP 1990
#define ELEV_DUTY_PITCH_DOWN 800


class PWMController
{
private:
	int channel; 
    unsigned long usedchannel; 
    unsigned long period, duty, count, currentThrust; 

	int currentElevPercent;
		
	int currentThrustPercent;
	int currentPitchAngle;

	bool pitchingForward, pitchingBack, yawingLeft, yawingRight, hovering, braking;
	
	int pitchIntensity, yawIntensity;

	int currentPitch, targetPitch;
	int currentAzimuth, targetAzimuth;

	void setElevPositionPitchUp();
	void setElevPositionPitchDown();
	void setElevPositionPitchNeutral();
	void sendDutyCycleToElev();

	void determineLeftOrRight(int currentAzimuth, int targetAzimuth, bool &left, bool &right);
	
public:
	PWMController();
	void arm_motors();

	void yaw_left();
	void yaw_right();

	void pitch_forward();
	void pitch_back();

	void start_yaw_left(bool startIfTrue_StopIfFalse, int intensity);	//must include motors and rudders
	void start_yaw_right(bool startIfTrue_StopIfFalse, int intensity); //must include motors and rudders
	
	void start_pitch_forward(bool startIfTrue_StopIfFalse, int intensity);  
	void start_pitch_backward(bool startIfTrue_StopIfFalse, int intensity);

	void start_braking(bool startIfTrue_StopIfFalse);

	void goToNeutral();
	
	

	void tick();

	int getThrustMagnitudePercent(){return currentThrustPercent;}
	void setThrustMagnitude(int percent_0_to_100);
	void setThrustVector(int angle_0_to_360);
	void genericTest(int servoNumber, int inDuty);
	void genericTest(int servoNumber, int inDuty, int inCount);
	
	void setTargetPitch(int inAngle){targetPitch=inAngle;}
	void setTargetAzimuth(int inAngle){targetAzimuth=inAngle;}
	int getTargetPitch(){return targetPitch;}
	void adjustPitch(int inCurrentPitch, double pitchVelocity);
	void adjustYaw(int inCurrentYaw, double yawVelocity);
	void setHover(bool trueOrFalse){hovering=true;}

	void speedUp(int percent_0_to_100);
	void speedDown(int percent_0_to_100);

	

	~PWMController();
};

#endif