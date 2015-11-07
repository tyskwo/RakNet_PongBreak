#pragma once
#include "RakNetTypes.h"
#include <vector>

struct PlayerState
{
	typedef float position;
	position mX;
	position mY;
};

struct PlayerInfo
{
	typedef PlayerInfo			player_info;
	typedef RakNet::RakNetGUID  guid;
	typedef float				position;

	PlayerInfo();
	PlayerInfo(PlayerState state, guid guid);

private:
	PlayerState mState;
	guid mGuid;

public:
	const PlayerState& GetState()			{ return mState; }
	void SetState(const PlayerState& state) { mState = state; }

	const guid& GetGuid() { return mGuid; }
	auto SetGuid(const guid& aGuid)->player_info&;
};

class PlayerInfoBuffer
{
public:
	typedef PlayerInfoBuffer         player_buffer;
	typedef PlayerInfo               player_info;
	typedef std::vector<player_info> player_info_container;
	typedef double                   second;

	PlayerInfoBuffer();
	PlayerInfoBuffer(player_info aStarting);

	player_info   GetNext(second aDeltaT) const;
	void          AddTarget(player_info aTarget);

private:
	second mTotalTimeToInterpolate;

	mutable player_info				mStarting;
	mutable player_info				mCurrent;
	mutable player_info_container	mTargets;
	mutable second					mCurrentTime;

public:
	const player_info& GetStartingInfo() { return mStarting; }
	const player_info& GetCurrentInfo()  { return mCurrent; }

	inline void SetTotalTimeToInterpolate(second value) { mTotalTimeToInterpolate = value; }

	auto SetStartingInfo(const player_info& info)->player_buffer&;
};