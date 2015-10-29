#ifndef SERVER_H
#define SERVER_H

#include <assert.h>
#include <iostream>

#include "BitStream.h"
#include "Gets.h"
#include "Kbhit.h"
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

class Server
{
public:
	Server();
	Server(const char* serverPort);
	~Server();

	void init(const char* serverPort);
	void cleanup();

	void update();

	void sendPacket();

private:
	//pointer to server object
	RakNet::RakPeerInterface* mpServer;
	
	//Holds packets
	RakNet::Packet* p;

	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;

	//first client
	RakNet::SystemAddress clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	//message to send to client
	char mMessage[2048];


	void getPackets();

	unsigned char GetPacketIdentifier(RakNet::Packet *p);
};

#endif