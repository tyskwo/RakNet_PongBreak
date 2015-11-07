#ifndef CLIENT_H
#define CLIENT_H

#include "RakPeerInterface.h"
#include "../common/Timer.h"
#include <array>

#pragma pack(push, 1)
struct ShapePosition
{
	unsigned char mID;
	float xPos, yPos, velocity;
};
#pragma pack(pop)

//struct for player values
struct Player
{
	float xPos, yPos;
	float xVel, yVel;

	std::array<std::array<bool, 3>, 6> bricks;

	int goalsScored;
};

//struct for ball values
struct Ball
{
	float xPos, yPos;
	float xVel, yVel;
};

//struct to receive game state from server
#pragma pack(push, 1)
struct GameInfo
{
	unsigned char mID;

	Player lPlayer, rPlayer;
	Ball ball;
};
#pragma pack(pop)

class Client
{
public:
	Client();
	Client(const char* clientPort, const char* serverAddress, const char* serverPort);
	~Client();

	void init(const char* clientPort, const char* serverAddress, const char* serverPort);
	void cleanup();

	void update();
	
	//send paddle data
	void sendPaddleData(float x, float y, float velocity);

	//values for the other player's shape position and velocity
	//########Need to convert to player struct##############
	float otherShapeX, otherShapeY, otherVelocity;

	//set flags for whether it was first connected, or connected second
	inline void setFirstConnected(bool wasFirst) { mWasFirstConnected = wasFirst; };
	inline bool getFirstConnected()			     { return mWasFirstConnected; };
	inline void setConnected(bool wasSecond)	 { mIsConnected = wasSecond; };
	inline bool getConnected()					 { return mIsConnected; };


private:
	//pointer to client object
	RakNet::RakPeerInterface* mpClient;

	//Holds packets
	RakNet::Packet* mpPacket;

	//first client
	//########Do we need these?#########
	RakNet::SystemAddress mClientID;
	RakNet::SocketDescriptor mSocketDescriptor;

	//get and identify packet
	unsigned char mPacketIdentifier;
	unsigned char GetPacketIdentifier(RakNet::Packet* pPacket);
	void getPackets();

	//values for the connection values.
	char mIPaddress[64], mServerPort[3], mClientPort[3];
	char mMessage[2048];

	//flag for if client is connected, and if so, first player
	bool mIsConnected;
	bool mWasFirstConnected;

	//timer
	Timer* mpTimer;
};

#endif