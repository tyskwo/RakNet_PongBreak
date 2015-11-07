#include "Timer.h"

Timer::Timer()
{
	mRakNetFrameTime = 1.0 / 30.0; //30 fps
	mDeltaT = 0.0; //no time has elapsed yet

	QueryPerformanceFrequency(&mFrequency); //get frequency
	TIMER_START //start timer
}

Timer::~Timer() {}

double Timer::calcDifferenceInMS()
{
	double difference = (double)(mEndTime.QuadPart - mStartTime.QuadPart) / (double)(mFrequency.QuadPart);
	return difference * 1000;
}

bool Timer::shouldUpdate()
{
	TIMER_STOP //end timer

	addDelta(calcDifferenceInMS()); //calculate and add elapsed time
	if (mDeltaT >= mRakNetFrameTime) //if it is time to update raknet
	{
		mDeltaT = mDeltaT - mRakNetFrameTime; //don't forget excess time
		return true; //return true
	}
	return false; //else return false
}