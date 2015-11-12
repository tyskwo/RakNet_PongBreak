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

struct Point
{
	float x, y;
};

//#pragma pack(push, 1)
//struct for player values
struct Player
{
	float x, y;
	std::array<std::array<bool, 3>, 6> bricks;
	std::array<Point, 18> brickLocs;

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
	bool started = false;

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
	struct Rectangle
	{
		float leftX, rightX, topY, bottomY;
		float width, height;
	};

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

	bool doesCollide(const Rectangle& rect1, const Rectangle& rect2);

	//timer
	Timer* mpTimer;
	
	//consts
	const double PI = std::atan(1.0) * 4;
	const float SCREEN_WIDTH = 1024.0;
	const float SCREEN_HEIGHT = 768.0;
	const float HALF_SCREEN_WIDTH = SCREEN_WIDTH / 2;
	const float HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;
};

#endif