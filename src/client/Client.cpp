#include "Client.h"

#include "MessageIdentifiers.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "PacketLogger.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <iostream>
#include "RakNetTypes.h"
#include "Kbhit.h"
#include "WindowsIncludes.h" // Sleep
#include "Gets.h"

#if LIBCAT_SECURITY==1
#include "SecureHandshake.h" // Include header for secure handshake
#endif


enum MessageTypes
{
	// For the user to use.  Start your first enumeration at this value.
	//ID_USER_PACKET_ENUM

	ID_SEND_PADDLE_DATA = ID_USER_PACKET_ENUM,
	ID_RECIEVE_PADDLE_DATA,
	ID_FIRST_CONNECTION,
	ID_SECOND_CONNECTION,
	ID_SEND_GAME_INFO,
	ID_RECIEVE_GAME_INFO,
	ID_RECIEVE_BALL_INFO
};

Client::Client()
: mOpponentInterpolation()
, mBallInterpolation()
{
	puts("Enter IP to connect to:");
	char temp[32];
	Gets(temp, sizeof(temp));
	Gets(mIPaddress, sizeof(mIPaddress));
	if (mIPaddress[0] == 0)
		strcpy_s(mIPaddress, "127.0.0.1");

	init("202", mIPaddress,"200");
}

Client::Client(const char* clientPort, const char* serverAddress, const char* serverPort)
: mOpponentInterpolation()
, mBallInterpolation()
{
	init(clientPort, serverAddress, serverPort);
}

Client::~Client()
{
	cleanup();
}

void Client::init(const char* clientPort, const char* serverAddress, const char* serverPort)
{
	mX = 0;
	mY = 0;
	ballX = 400;
	ballY = 400;

	mIsConnected = false;

	//create client instance
	mpClient = RakNet::RakPeerInterface::GetInstance();

	mClientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	mpClient->AllowConnectionResponseIPMigration(false);


	// Connecting the client is very simple.  0 means we don't care about
	// a connectionValidationInteger, and false for low priority threads
	mSocketDescriptor = RakNet::SocketDescriptor(atoi(clientPort), 0);
	mSocketDescriptor.socketFamily = AF_INET;
	mpClient->Startup(8, &mSocketDescriptor, 1);
	mpClient->SetOccasionalPing(true);


	RakNet::ConnectionAttemptResult car = mpClient->Connect(serverAddress, atoi(serverPort), "hello", (int)strlen("hello"));
	RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);

	mpTimer = new Timer();
}

void Client::cleanup()
{
	// Be nice and let the server know we quit.
	mpClient->Shutdown(300);
	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(mpClient);
}

unsigned char Client::GetPacketIdentifier(RakNet::Packet *p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char)p->data[0];
}

void Client::update()
{
	// Get a packet from either the server or the client
	getPackets();

	//if enough time has passed (30fps), broadcast game states to clients
	if (mpTimer->shouldUpdate())
	{
		sendPaddleData(mX, mY);
	}
	else
	{

	}
}

void Client::getPackets()
{
	// Get a packet from either the server or the client
	for (mpPacket = mpClient->Receive(); mpPacket; mpClient->DeallocatePacket(mpPacket), mpPacket = mpClient->Receive())
	{
		// We got a packet, get the identifier with our handy function
		mPacketIdentifier = GetPacketIdentifier(mpPacket);

		// Check if this is a network message packet
		switch (mPacketIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally
			printf("ID_DISCONNECTION_NOTIFICATION\n");
			break;
		case ID_ALREADY_CONNECTED:
			// Connection lost normally
			printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", mpPacket->guid);
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
			break;
		case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_CONNECTION_LOST\n");
			break;
		case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
			break;
		case ID_CONNECTION_BANNED: // Banned from this server
			printf("We are banned from this server.\n");
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Connection attempt failed\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			// Sorry, the server is full.  I don't do anything here but
			// A real app should tell the user
			printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
			break;

		case ID_INVALID_PASSWORD:
			printf("ID_INVALID_PASSWORD\n");
			break;

		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST\n");
			break;

		case ID_CONNECTION_REQUEST_ACCEPTED:
			// This tells the client they have connected
			//printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", mpPacket->systemAddress.ToString(true), mpPacket->guid.ToString());
			//printf("My external address is %s\n", mpClient->GetExternalID(mpPacket->systemAddress).ToString(true));
			//setConnected(true);
			break;
		case ID_CONNECTED_PING:
			break;
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", mpPacket->systemAddress.ToString(true));
			break;



		//########################################USER CHANGED IDs################################################

		case ID_FIRST_CONNECTION:
		{
			//set as first connected or second connected.
			setFirstConnected(true);
			setConnected(true);
			std::cout << "first" << std::endl;
			break;
		}
		case ID_SECOND_CONNECTION:
		{
			//set as first connected or second connected.
			setFirstConnected(false);
			setConnected(true);
			std::cout << "second" << std::endl;
			break;
		}

		case ID_RECIEVE_PADDLE_DATA:
		{
			ShapePosition pos = *reinterpret_cast<ShapePosition*>(mpPacket->data);
			//otherShapeX = pos.xPos;
			//otherShapeY = pos.yPos;
			//otherVelocity = pos.velocity;

			ObjectState pState;
			pState.mX = pos.xPos;
			pState.mY = pos.yPos;
			ObjectInfo info;
			info.SetState(pState);

			mOpponentInterpolation.AddTarget(info);
			break;
		}

		case ID_RECIEVE_BALL_INFO:
		{
			Ball theBall = *reinterpret_cast<Ball*>(mpPacket->data);
			//ballX = theBall.x;
			//ballY = theBall.y;

			ObjectState pState;
			pState.mX = ballX;
			pState.mY = ballY;
			ObjectInfo info;
			info.SetState(pState);

			mBallInterpolation.AddTarget(info);
			break;
		}

		default:
			//try to print data
			printf("%s\n", mpPacket->data);
			break;
		}
	}
}

void Client::sendPaddleData(float x, float y)
{
	ShapePosition pos;
	pos.xPos = x;
	pos.yPos = y;
	pos.velocity = 0;
	pos.mID = ID_SEND_PADDLE_DATA;
	mpClient->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}
