#ifndef CLIENT_H
#define CLIENT_H

#include "RakPeerInterface.h"

#pragma pack(push, 1)
struct ShapePosition
{
	unsigned char mID;
	float xPos, yPos;
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
	void sendShapePacket(float x, float y);

	float otherShapeX, otherShapeY;

	inline void setFirstConnected(bool wasFirst) { mWasFirstConnected = wasFirst; };
	inline bool getFirstConnected()			     { return mWasFirstConnected; };


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

	bool mWasFirstConnected;
};

#endif