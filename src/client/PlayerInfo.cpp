#include "PlayerInfo.h"

PlayerInfo::PlayerInfo() {}

PlayerInfo::PlayerInfo(PlayerState state, guid guid)
: mState(state)
, mGuid(guid)
{}

auto PlayerInfo::SetGuid(const guid& aGuid) -> player_info&
{
	mGuid = aGuid;
	return *this;
}

PlayerInfoBuffer::PlayerInfoBuffer()
: mTotalTimeToInterpolate(1.0 / 10.0)
, mCurrentTime(0)
{}

PlayerInfoBuffer::PlayerInfoBuffer(player_info aStarting)
: PlayerInfoBuffer()
{
	mStarting = aStarting;
	mCurrent =  mStarting;
}

auto PlayerInfoBuffer::GetNext(second aDeltaT) const -> player_info
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

	PlayerState pState;
	pState.mX = static_cast<float>(xVal);
	pState.mY = static_cast<float>(yVal);

	mCurrent.SetState(pState);
	mCurrent.SetGuid(mStarting.GetGuid());

	return PlayerInfo(pState, mStarting.GetGuid());
}

void PlayerInfoBuffer::AddTarget(player_info aTarget)
{
	mTargets.push_back(aTarget);
}

auto PlayerInfoBuffer::SetStartingInfo(const player_info& info) -> player_buffer&
{
	mCurrentTime = 0.0;
	mStarting    = info;
	mCurrent     = mStarting;
	return *this;
}