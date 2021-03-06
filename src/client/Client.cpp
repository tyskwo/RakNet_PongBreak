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
	ID_RECIEVE_GAME_INFO,
	ID_RECIEVE_BALL_INFO, //not used by client
	ID_START_GAME
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
	mIsConnected = false;

	//create client instance
	mpClient = RakNet::RakPeerInterface::GetInstance();
	mpClient->AllowConnectionResponseIPMigration(false);


	// Connecting the client is very simple.  0 means we don't care about
	// a connectionValidationInteger, and false for low priority threads
	mSocketDescriptor = RakNet::SocketDescriptor(atoi(clientPort), 0);
	mSocketDescriptor.socketFamily = AF_INET;
	mpClient->Startup(8, &mSocketDescriptor, 1);
	mpClient->SetOccasionalPing(true);


	RakNet::ConnectionAttemptResult car = mpClient->Connect(serverAddress, atoi(serverPort), "hello", (int)strlen("hello"));
	RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);

	mSendGameStart = false;
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
		if (getFirstConnected())
		{
			sendPaddleData(mGameInfo.lPlayer.x, mGameInfo.lPlayer.y);
		}
		else
		{
			sendPaddleData(mGameInfo.rPlayer.x, mGameInfo.rPlayer.y);
		}

		if (mSendGameStart)
		{
			sendGameStart();
			mSendGameStart = false;
		}
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

			GameInfo gameInfo = *reinterpret_cast<GameInfo*>(mpPacket->data);
			mGameInfo = gameInfo;

			ObjectState ballState;
			ballState.mX = gameInfo.ball.x;
			ballState.mY = gameInfo.ball.y;
			ObjectInfo ballInfo;
			ballInfo.SetState(ballState);
			mBallInterpolation.SetStartingInfo(ballInfo);

			ObjectState otherState;
			otherState.mX = gameInfo.rPlayer.x;
			otherState.mY = gameInfo.rPlayer.y;
			ObjectInfo otherInfo;
			otherInfo.SetState(otherState);
			mOpponentInterpolation.SetStartingInfo(otherInfo);

			std::cout << "FIRST" << std::endl;

			break;
		}
		case ID_SECOND_CONNECTION:
		{
			//set as first connected or second connected.
			setFirstConnected(false);
			setConnected(true);

			GameInfo gameInfo = *reinterpret_cast<GameInfo*>(mpPacket->data);
			mGameInfo = gameInfo;

			ObjectState ballState;
			ballState.mX = gameInfo.ball.x;
			ballState.mY = gameInfo.ball.y;
			ObjectInfo ballInfo;
			ballInfo.SetState(ballState);
			mBallInterpolation.SetStartingInfo(ballInfo);

			ObjectState otherState;
			otherState.mX = gameInfo.lPlayer.x;
			otherState.mY = gameInfo.lPlayer.y;
			ObjectInfo otherInfo;
			otherInfo.SetState(otherState);
			mOpponentInterpolation.SetStartingInfo(otherInfo);

			std::cout << "SECOND" << std::endl;

			break;
		}

		case ID_RECIEVE_GAME_INFO:
		{
			GameInfo gameInfo = *reinterpret_cast<GameInfo*>(mpPacket->data);

			ObjectState ballState;
			ballState.mX = gameInfo.ball.x + gameInfo.ball.xVel;
			ballState.mY = gameInfo.ball.y + gameInfo.ball.yVel;

			ObjectInfo ballInfo;
			ballInfo.SetState(ballState);

			ObjectState ballStartState;
			ballStartState.mX = mGameInfo.ball.x;
			ballStartState.mY = mGameInfo.ball.y;

			ObjectInfo ballStartInfo;
			ballStartInfo.SetState(ballStartState);
			mBallInterpolation.SetStartingInfo(ballStartInfo);


			if (mGameInfo.ball.x != gameInfo.ball.x && mGameInfo.ball.y != gameInfo.ball.y)
			{
				mBallInterpolation.AddTarget(ballInfo);
			}
			mGameInfo.ball.xVel = gameInfo.ball.xVel;
			mGameInfo.ball.yVel = gameInfo.ball.yVel;

			mGameInfo.lPlayer.bricks = gameInfo.lPlayer.bricks;
			mGameInfo.rPlayer.bricks = gameInfo.rPlayer.bricks;

			mGameInfo.lPlayer.goalsScored = gameInfo.lPlayer.goalsScored;
			mGameInfo.rPlayer.goalsScored = gameInfo.rPlayer.goalsScored;

			mGameInfo.ball.collided = gameInfo.ball.collided;

			mGameInfo.started = gameInfo.started;
			mGameInfo.finished = gameInfo.finished;

			if (getFirstConnected())
			{
				ObjectState opponentState;
				opponentState.mX = gameInfo.rPlayer.x;
				opponentState.mY = gameInfo.rPlayer.y + gameInfo.rPlayer.velocity;
				ObjectInfo opponentInfo;
				opponentInfo.SetState(opponentState);

				ObjectState otherStartState;
				otherStartState.mX = mGameInfo.rPlayer.x;
				otherStartState.mY = mGameInfo.rPlayer.y;

				ObjectInfo otherStartInfo;
				otherStartInfo.SetState(otherStartState);
				mOpponentInterpolation.SetStartingInfo(otherStartInfo);


				if (mGameInfo.rPlayer.y != gameInfo.rPlayer.y)
				{
					mOpponentInterpolation.AddTarget(opponentInfo);
				}
			}
			else
			{
				ObjectState opponentState;
				opponentState.mX = gameInfo.lPlayer.x;
				opponentState.mY = gameInfo.lPlayer.y + gameInfo.lPlayer.velocity;
				ObjectInfo opponentInfo;
				opponentInfo.SetState(opponentState);

				ObjectState otherStartState;
				otherStartState.mX = mGameInfo.lPlayer.x;
				otherStartState.mY = mGameInfo.lPlayer.y;

				ObjectInfo otherStartInfo;
				otherStartInfo.SetState(otherStartState);
				mOpponentInterpolation.SetStartingInfo(otherStartInfo);


				if (mGameInfo.lPlayer.y != gameInfo.lPlayer.y)
				{
					mOpponentInterpolation.AddTarget(opponentInfo);
				}
			}
			mGameInfo.lPlayer.velocity = gameInfo.lPlayer.velocity;
			mGameInfo.rPlayer.velocity = gameInfo.rPlayer.velocity;


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
	Position pos;
	pos.x = x;
	pos.y = y;
	pos.mID = ID_SEND_PADDLE_DATA;
	mpClient->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::setPaddleLoc(const float& x, const float& y)
{
	if (mWasFirstConnected) mGameInfo.lPlayer.x = x, mGameInfo.lPlayer.y = y;
	else mGameInfo.rPlayer.x = x, mGameInfo.rPlayer.y = y;
}

void Client::setOpponentLoc(const float& x, const float& y)
{
	if (mWasFirstConnected) mGameInfo.rPlayer.x = x, mGameInfo.rPlayer.y = y;
	else mGameInfo.lPlayer.x = x, mGameInfo.lPlayer.y = y;
}

void Client::sendGameStart()
{
	int mID = ID_START_GAME;
	mpClient->Send((const char*)&mID, sizeof(mID), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void Client::setBallPosition(const float& x, const float& y)
{
	mGameInfo.ball.x = x;
	mGameInfo.ball.y = y;
}

void Client::setPaddleVelocity(const float& velocity)
{
	if (mWasFirstConnected) mGameInfo.lPlayer.velocity = velocity;
	else mGameInfo.rPlayer.velocity = velocity;
}