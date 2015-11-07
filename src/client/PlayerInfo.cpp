#include "PlayerInfo.h"

PlayerInfo::
PlayerInfo()
{ }

PlayerInfo::
PlayerInfo(PlayerState state, guid_type guid)
: m_state(state)
, m_guid(guid)
{ }

auto
PlayerInfo::
SetGuid(const guid_type& a_guid) -> this_type&
{
	m_guid = a_guid;
	return *this;
}

PlayerInfoPair::
PlayerInfoPair()
: m_totalTimeToInter(1000.0 / 1.0)
, m_currTime(0)
{ }

PlayerInfoPair::
PlayerInfoPair(player_info a_starting)
: PlayerInfoPair()
{
	m_starting = a_starting;
	m_current = m_starting;
}

auto
PlayerInfoPair::
GetNext(sec_type a_deltaT) const
-> player_info
{
	if (m_targets.size() == 0)
	{
		return m_starting;
	}

	m_currTime += a_deltaT;

	if (m_currTime >= m_totalTimeToInter)
	{
		m_currTime = 0.0;
		m_starting = m_targets.front();
		m_current = m_starting;
		m_targets.erase(m_targets.begin());

		return m_current;
	}

	const auto t = m_currTime / m_totalTimeToInter;

	auto startState = m_starting.GetState();
	auto targetState = m_targets.front().GetState();

	auto xVal = startState.m_x * (1.0 - t) + targetState.m_x * t;
	auto yVal = startState.m_y * (1.0 - t) + targetState.m_y * t;

	PlayerState pState;
	pState.m_x = static_cast<float>(xVal);
	pState.m_y = static_cast<float>(yVal);

	m_current.SetState(pState);
	m_current.SetGuid(m_starting.GetGuid());

	return PlayerInfo(pState, m_starting.GetGuid());
}

void
PlayerInfoPair::
AddTarget(player_info a_target)
{
	m_targets.push_back(a_target);
}

auto
PlayerInfoPair::
SetStartingInfo(const player_info& info)
-> this_type&
{
	m_currTime = 0.0;
	m_starting = info;
	m_current = m_starting;
	return *this;
}