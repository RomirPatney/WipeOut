void parking (int const target, int const range, int  gain, int  power)
{
eraseDisplay();
displayString(0,"Parking");
	motor[motorA]=-20;

	motor[motorC]=-20;
	wait1Msec(3000);
	nMotorEncoder[motorC]=0;

	motor[motorA]=10;
	motor[motorC]=-19;
	while(nMotorEncoder[motorC]>-460){}

	motor[motorA]=-8;
	motor[motorC]=-8;
	while(SensorValue[S2]<target){}


	while (SensorValue[S2]<target)
	{
		lineFollowingTaxi(target, range, 50, 10);
	}

		nMotorEncoder[motorA]=0;
		nMotorEncoder[motorC]=0;

	while( nMotorEncoder[motorA]<300 || nMotorEncoder[motorC]<300)
	{
		lineFollowingTaxi(target, range, 30, 10);
	}

	motor[motorA]=0;
	motor[motorC]=0;
	eraseDisplay();
}
