#pragma once
#include "RakNetTypes.h"
#include <vector>

struct PlayerState
{
	typedef float		pos_type;
	pos_type m_x;	
	pos_type m_y;
};

struct PlayerInfo
{
	typedef PlayerInfo						this_type;
	typedef RakNet::RakNetGUID              guid_type;
	typedef float							pos_type;

	PlayerInfo();
	PlayerInfo(PlayerState state, guid_type guid);

private:
	PlayerState m_state;
	guid_type m_guid;

public:
	const PlayerState& GetState() { return m_state; }
	void SetState(const PlayerState& state) { m_state = state; }

	const guid_type& GetGuid() { return m_guid; }
	auto SetGuid(const guid_type& a_guid)->this_type&;
};

class PlayerInfoPair
{
public:
	typedef PlayerInfoPair                      this_type;
	typedef PlayerInfo                          player_info;
	typedef std::vector<player_info>		    player_info_cont;
	typedef double                              sec_type;

	PlayerInfoPair();
	PlayerInfoPair(player_info a_starting);

	player_info   GetNext(sec_type a_deltaT) const;
	void          AddTarget(player_info a_target);

private:
	sec_type          m_totalTimeToInter;

	mutable player_info       m_starting;
	mutable player_info       m_current;
	mutable player_info_cont  m_targets;
	mutable sec_type          m_currTime;

public:
	const player_info& GetStartingInfo() { return m_starting; }
	const player_info& GetCurrentInfo() { return m_current; }

	void SetTotalTimeToInterpolate(sec_type value) { m_totalTimeToInter = value; }

	auto SetStartingInfo(const player_info& info)->this_type&;

	//TLOC_DECL_SETTER_CHAIN(player_info, SetStartingInfo);

	//TLOC_DECL_AND_DEF_CONTAINER_ALL_METHODS(_targets, m_targets);
};