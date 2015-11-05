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

#pragma pack(push, 1)
struct ShapePosition
{
	unsigned char mID;
	float xPos, yPos;
};
#pragma pack(pop)

struct Player
{
	float xPos, yPos;
	float xVel, yVel;

	std::array<std::array<bool, 3>, 6> bricks;

	int goalsScored;
};

struct Ball
{
	float xPos, yPos;
	float xVel, yVel;
};

#pragma pack(push, 1)
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

	//get packet
	void getPackets();

	//determine id of packet
	unsigned char packetIdentifier;
	unsigned char GetPacketIdentifier(RakNet::Packet *p);

	//arrays for the games, and info for said games
	std::array<std::array<RakNet::SystemAddress, 2>, 8> mClientPairs;
	std::array<GameInfo, 8>								mGameInfos;
	int mNumGames;
	
	//message to send to client
	//MIGHT NOT BE NEEDED IN END.
	char mMessage[2048];

	//timer variables
	LARGE_INTEGER mStartTime;
	LARGE_INTEGER mEndTime;
	LARGE_INTEGER mFrequency;
	double Server::calcDifferenceInMS(LARGE_INTEGER from, LARGE_INTEGER to) const;
};

#endif