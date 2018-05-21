#include "NXT_FileIO.c"
bool linefoInit = false;

int timeSchedule [5][2];
//**********************************************************

void clearAllMotorEncoder ()
{
	nMotorEncoder[motorA] = 0;
	nMotorEncoder[motorB] = 0;
	nMotorEncoder[motorC] = 0;
}
//***********************************************************

bool readFile ()
{
	TFileHandle fin;
	bool fileOkay = openReadPC(fin,"ATCSchedule.txt");

	if (!fileOkay)
	{
		displayString(0, "Cannot Open File:");
		displayString(1, "ATCSchedule.txt");
		displayString(2, "Program Abort");
		return false;
	}
	int prevTLTime =0;
	int tlTime = 0;
	int index= 0;

	while(readIntPC(fin, tlTime))
	{
		if (tlTime-prevTLTime>=60 && tlTime != -1 && index<5)
		{
			timeSchedule[index][0]=prevTLTime;
			timeSchedule[index][1]=tlTime;
			index++;
		}
		prevTLTime=tlTime;
	}

	if (index<=0)
	{
		displayString(0, "File Read Success.");
		displayString(1, "No Possible Interval.");
		displayString(2, "Airport is too busy");
		displayString(3, "Please Start Manually");
		return false;
	}

	displayString (0, "Possible");
	displayString (0, "Cleaning Time");
	for (int i=0 ; i<5; i++)
	{

		displayString(i+1, "%d\t\t\t%d", timeSchedule [i][0],timeSchedule [i][1]);
	}
	wait1Msec(5000);

	return true;
}

//***********************************************************
bool eStop(bool linefoInit)
{
	bool status = false;
	if (SensorValue[S3] == 1 || SensorValue[S4] < 40)
	{
		motor[motorA] = 0;
		motor[motorB] = 0;
		motor[motorC] = 0;
		status = true;
		linefoInit = false;
	}
	return status;
}

//*****************************************************************************
#include "sweeperAngle.c"
//*******************************************************************************
void checkSchedule (int & nextDepart, int const cycleComplete)
{
	bool foundTime=false;
	time1[T4]=0;

	if (cycleComplete!=0)
	{
		while ((time1[T4])<(5000))
		{
			displayString(0, "waiting-(runway");
			displayString(1,"just cleaned)...");
		}
	}

	eraseDisplay();
	displayString(0,"  Checking ");
	displayString(1," Schedule... ");
	//now check for next appropriate time to depart
	for (int i=0; i<5 && foundTime == false;i++)
	{
		if ((timeSchedule[i][1]-timeSchedule[i][0])>=60 && ((time1[T4]+cycleComplete)/1000)<timeSchedule[i][0])
		{
			nextDepart = timeSchedule[i][0]*1000;
			foundTime = true;
		}
		else if (timeSchedule[i][1]-((time1[T4]+cycleComplete)/1000)>(60+10))
		{
			nextDepart = ((time1[T4]+cycleComplete)+10000);
			foundTime = true;
		}
	}
	eraseDisplay();
	displayString(0, "Found a Time!");
	playSound(soundFastUpwardTones);
	wait1Msec(1000);
	eraseDisplay();
}

//*******************************************************************************

void lineCal (int & range, int & target, bool linefoInit)
{
	bool linefoInit = false;

	eraseDisplay();
	displayString(0,"Initialization...");
	int max=0, min=999;

	nMotorEncoder[motorA]=0;
	motor[motorA]=10;
	motor[motorC]=-10;
	while(nMotorEncoder[motorA]<=60)
	{
		if (SensorValue[S2]>max)
			max=SensorValue[S2];
		if (SensorValue[S2]<min)
			min=SensorValue[S2];
	}


	nMotorEncoder[motorA]=0;
	motor[motorA]=-10;
	motor[motorC]=10;
	while(nMotorEncoder[motorA]>=-130)
	{
		if (SensorValue[S2]>max)
			max=SensorValue[S2];
		if (SensorValue[S2]<min)
			min=SensorValue[S2];
	}


	range=max-min;
	target=(range/2)+min;


	motor[motorA]=10;
	motor[motorC]=10;
	while(SensorValue[S2]!=target){} //be at the edge

	/*
	motor[motorA]=10;
	motor[motorC]=15;
	wait1Msec(600);

	if (SensorValue[S2]>target) //sensor see more white, sensorValue is incresing, at left edge
	{
	motor[motorA]=10; // cross the line
	motor[motorC]=10;
	while(SensorValue[S2]!=target){displayString(0,"LineCal");}
	}
	*/

	motor[motorA]=0;
	motor[motorC]=0;
	displayString(0,"LineCal LAST");
}



void lineFollowingTaxi (int const target, int const range, int const gain, int const power, bool linefoInit)
{
	int correction = 0;

	if (!linefoInit)
	{
		motor[motorA]=power;
		motor[motorC]=power;
		linefoInit=true;
	}
	else if(abs(target-SensorValue[S2])>2)
	{
		correction=gain*(target-SensorValue[S2])/range;

		motor[motorA]=power+correction;
		motor[motorC]=power-correction;
	}
}

void lineFollowingRunway (int target, int const range, int const gain, int const power, bool linefoInit)
{
	int correction=0;

	if (!linefoInit)
	{
		motor[motorA]=power;
		motor[motorC]=power;
		linefoInit=true;
	}
	else if(abs(target-SensorValue[S2])>5)
	{
		correction=gain*(target-SensorValue[S2]+5)/range;

		motor[motorA]=power+correction;
		motor[motorC]=power-correction;
	}
}


void lineFollowing (int const target, int const range, int const gain, int const power)
{

	if (SensorValue[S1]!=6)
	{
		lineFollowingTaxi (target, range, 55, 25 );
	}

	else
	{
		lineFollowingRunway (target, range, 40, 40);
	}

}

#include "parking.c"
//*******************************************************************************


//////////////////////////////////////////MAIN
//////////////////////////////////////////MAIN
//////////////////////////////////////////MAIN
//////////////////////////////////////////MAIN
task main()
{

	//sensor setup
	SensorType[S1]=sensorColorNxtFULL;
	SensorType[S2]=sensorLightActive;
	SensorType[S3]=sensorTouch;
	SensorType[S4]=sensorSONAR;

	//variable declearation
	int power=30;
	int gain=60;
	int range, target=0;

	//sweeper angle
	int previousAngle = 0;
	bool offseted = false;

	int nextDepart=0, cycleComplete=0;

	time1[T1]=0; // T1 : master clock;

	playSound(soundBeepBeep);

	clearAllMotorEncoder();

	readFile ();

	while(time1[T1]<401000) // main loop
	{
		checkSchedule(nextDepart, cycleComplete);
		displayString(0, "Next Depart At:");
		displayString(1, "\t\t\t\t\t%d", nextDepart);
		while(time1[T1]<nextDepart && nNxtButtonPressed != 3)
		{
			displayString(2, "Time Now: %d", time1[T1]);
		}

		eraseDisplay();
		displayString(0,"STAY CLEAR");
		displayString(1,"ABOUT TO DEPART");
		playTone(1000,500);
		wait1Msec(2000);

		eraseDisplay();

		if (cycleComplete==0)
		{
			lineCal(range, target);
		}

		sweeperAngle(-15, previousAngle, offseted);

		eraseDisplay();
		while(SensorValue[S1]!=5)
		{
			if (!eStop(linefoInit))
			{
				displayString(0,"Clening...");
				lineFollowing(target,range,gain,power);
			}
			else
			{
				while (nNxtButtonPressed==-1)
				{
					playSound(soundBeepBeep);
					displayString(0,"EMERGENCY STOP");
				}
				eraseDisplay();
				clearSounds();
			}
		}
		sweeperAngle(0, previousAngle, offseted);
		parking (target, range, gain, power);

		motor[motorA]=0;
		motor[motorB]=0;

		linefoInit = false;
		cycleComplete=time1[T1];

		eraseDisplay();
		displayString(0,"Cycle Completed");
		wait1Msec(1000);
	}
	sweeperAngle(0, previousAngle, offseted);
	eraseDisplay();
	displayString(0,"Night Curfew");
	displayString(1,"All Finished");
	playSound(soundDownwardTones);
	wait1Msec(7000);
}
