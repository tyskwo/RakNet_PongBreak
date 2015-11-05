#ifndef CLIENT_H
#define CLIENT_H

#include "RakPeerInterface.h"
#include <array>

#pragma pack(push, 1)
struct ShapePosition
{
	unsigned char mID;
	float xPos, yPos, velocity;
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
	
	void sendPacket();
	void sendPaddleData(float x, float y, float velocity);

	float otherShapeX, otherShapeY, otherVelocity;

	inline void setFirstConnected(bool wasFirst) { mWasFirstConnected = wasFirst; };
	inline bool getFirstConnected()			     { return mWasFirstConnected; };

	inline void setConnected(bool connected) { mIsConnected = connected; };
	inline bool getConnected()			     { return mIsConnected; };


private:
	//pointer to client object
	RakNet::RakPeerInterface* mpClient;

	//Holds packets
	RakNet::Packet* mpPacket;

	//first client
	RakNet::SystemAddress mClientID;
	RakNet::SocketDescriptor mSocketDescriptor;

	// GetPacketIdentifier returns this
	unsigned char mPacketIdentifier;

	void getPackets();

	unsigned char GetPacketIdentifier(RakNet::Packet* pPacket);

	char mIPaddress[64], mServerPort[3], mClientPort[3];
	char mMessage[2048];

	bool mIsConnected;
	bool mWasFirstConnected;
};

#endif