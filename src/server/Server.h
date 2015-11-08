#ifndef SERVER_H
#define SERVER_H

#include <assert.h>
#include <iostream>
#include <array>
#include <time.h>

#include "BitStream.h"
#include "Gets.h"
#include "MessageIdentifiers.h"
#include "PacketLogger.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include "RakSleep.h"
#include "../common/Timer.h"

#pragma pack(push, 1)
struct PaddleData
{
	unsigned char mID;
	float x, y;
};
#pragma pack(pop)

//#pragma pack(push, 1)
//struct for player values
struct Player
{
	float x, y;
	std::array<std::array<bool, 3>, 6> bricks;

	int goalsScored;
};
//#pragma pack(pop)

//#pragma pack(push, 1)
//struct for ball values
struct Ball
{
	float x, y;
	float xVel, yVel;
};
//#pragma pack(pop)

#pragma pack(push, 1)
//struct for game info, both players, ball, scores
struct GameInfo
{
	unsigned char mID;

	Player lPlayer;
	Player rPlayer;
	Ball ball;
};
#pragma pack(pop)

class Server
{
public:
	Server();
	Server(const char* serverPort);
	~Server();

	void init(const char* serverPort);
	void cleanup();

	void update();
	void broadcastGameInfo();

private:
	//pointer to server object
	RakNet::RakPeerInterface* mpServer;
	
	//Holds packets
	RakNet::Packet* p;

	void getPackets();
	//determine id of packet
	unsigned char packetIdentifier;
	unsigned char GetPacketIdentifier(RakNet::Packet *p);

	//arrays for the games, and info for said games
	std::array<std::array<RakNet::SystemAddress, 2>, 8> mClientPairs;
	std::array<GameInfo, 4>								mGameInfos;
	int mNumGames;

	void initializeGameInfos();
	void updateGames();

	//timer
	Timer* mpTimer;

	const double PI = std::atan(1.0) * 4;
};

#endif