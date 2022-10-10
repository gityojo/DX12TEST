#pragma once

#include <windows.h>

class Timer
{
public:
	Timer();

	double TotalTime() const;
	double DeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();
private:
	double mSecondsPerCount;
	double mDeltaTime;

	LARGE_INTEGER mBaseTime;
	LARGE_INTEGER mPrevTime;
	LARGE_INTEGER mCurrTime;
	LARGE_INTEGER mStopTime;
	LARGE_INTEGER mStoppedTime;

	bool mStopped;
};
