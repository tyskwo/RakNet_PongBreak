#pragma once
#include "RakNetTypes.h"
#include <vector>

struct ObjectState
{
	typedef float position;
	position mX;
	position mY;
};

struct ObjectInfo
{
	typedef ObjectInfo object_info;
	typedef float	   position;

	ObjectInfo();
	ObjectInfo(ObjectState state);

private:
	ObjectState mState;

public:
	const ObjectState& GetState()			{ return mState; }
	void SetState(const ObjectState& state) { mState = state; }
};

class ObjectInfoBuffer
{
public:
	typedef ObjectInfoBuffer         object_buffer;
	typedef ObjectInfo               object_info;
	typedef std::vector<object_info> object_info_container;
	typedef double                   second;

	ObjectInfoBuffer();
	ObjectInfoBuffer(object_info aStarting);

	object_info   GetNext(second aDeltaT) const;
	void          AddTarget(object_info aTarget);

private:
	second mTotalTimeToInterpolate;

	mutable object_info				mStarting;
	mutable object_info				mCurrent;
	mutable object_info_container	mTargets;
	mutable second					mCurrentTime;

public:
	const object_info& GetStartingInfo() { return mStarting; }
	const object_info& GetCurrentInfo()  { return mCurrent; }

	inline void SetTotalTimeToInterpolate(second value) { mTotalTimeToInterpolate = value; }

	void SetStartingInfo(const object_info& info);
};