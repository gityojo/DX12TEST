#include "Timer.h"

Timer::Timer() : mSecondsPerCount(0.0), mDeltaTime(0.0), mStopped(false)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	mSecondsPerCount = 1.0 / (double)frequency.QuadPart;
}

double Timer::TotalTime() const
{
	if (mStopped)
	{
		return (mStopTime.QuadPart - mBaseTime.QuadPart - mStoppedTime.QuadPart) * mSecondsPerCount;
	}
	else
	{
		return (mCurrTime.QuadPart - mBaseTime.QuadPart - mStoppedTime.QuadPart) * mSecondsPerCount;
	}
}

double Timer::DeltaTime() const
{
	return mDeltaTime;
}

void Timer::Reset()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	mBaseTime = now;
	mPrevTime = now;
}

void Timer::Start()
{
	if (mStopped)
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		mStoppedTime.QuadPart += now.QuadPart - mStopTime.QuadPart;
		mPrevTime = now;
		mStopped = false;
	}
}

void Timer::Stop()
{
	if (!mStopped)
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		mStopTime = now;
		mStopped = true;
	}
}

void Timer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}

	QueryPerformanceCounter(&mCurrTime);

	mDeltaTime = (mCurrTime.QuadPart - mPrevTime.QuadPart) * mSecondsPerCount;
	mPrevTime = mCurrTime;
}
