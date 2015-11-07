#include "ObjectInfo.h"
#include <iostream>

ObjectInfo::ObjectInfo() {}

ObjectInfo::ObjectInfo(ObjectState state)
: mState(state)
{}

ObjectInfoBuffer::ObjectInfoBuffer()
: mTotalTimeToInterpolate(1000.0 / 1.0)
, mCurrentTime(0)
{}

ObjectInfoBuffer::ObjectInfoBuffer(object_info aStarting)
	: ObjectInfoBuffer()
{
	mStarting = aStarting;
	mCurrent =  mStarting;
}

auto ObjectInfoBuffer::GetNext(second aDeltaT) const -> object_info
{
	if (mTargets.size() == 0)
	{
		return mStarting;
	}

	mCurrentTime += aDeltaT;

	if (mCurrentTime >= mTotalTimeToInterpolate)
	{
		mCurrentTime = 0.0;
		mStarting    = mTargets.front();
		mCurrent     = mStarting;
		mTargets.erase(mTargets.begin());

		return mCurrent;
	}

	const auto time  = mCurrentTime / mTotalTimeToInterpolate;

	auto startState  = mStarting.GetState();
	auto targetState = mTargets.front().GetState();

	auto xVal = startState.mX * (1.0 - time) + targetState.mX * time;
	auto yVal = startState.mY * (1.0 - time) + targetState.mY * time;

	ObjectState pState;
	pState.mX = static_cast<float>(xVal);
	pState.mY = static_cast<float>(yVal);

	mCurrent.SetState(pState);

	return ObjectInfo(pState);
}

void ObjectInfoBuffer::AddTarget(object_info aTarget)
{
	mTargets.push_back(aTarget);
	std::cout << mTargets.size() << std::endl;
}

auto ObjectInfoBuffer::SetStartingInfo(const object_info& info) -> object_buffer&
{
	mCurrentTime = 0.0;
	mStarting    = info;
	mCurrent     = mStarting;
	return *this;
}