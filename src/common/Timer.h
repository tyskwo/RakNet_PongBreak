#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <windows.h>

class Timer
{
public:
	Timer();
	~Timer();

	//defines to start and stop timer
	#define TIMER_START QueryPerformanceCounter(&mStartTime);
	#define TIMER_STOP  QueryPerformanceCounter(&mEndTime);

	//returns true if enough time has passed
	bool   shouldUpdate();
	
private:
	//timer variables
	LARGE_INTEGER mStartTime;
	LARGE_INTEGER mFrequency;
	LARGE_INTEGER mEndTime;

	//how often RakNet should update and elapsed time
	double mRakNetFrameTime, mDeltaT;

	//add time
	inline void addDelta(double delta) { mDeltaT += delta; };
	//calc elapsed time
	double calcDifferenceInMS();
};

#endif