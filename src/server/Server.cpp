#include "Server.h"
#include <cmath>

enum MessageTypes
{
	// For the user to use.  Start your first enumeration at this value.
	//ID_USER_PACKET_ENUM,
	//-------------------------------------------------------------------------------------------------------------

	ID_SEND_PADDLE_DATA = ID_USER_PACKET_ENUM,
	ID_RECIEVE_PADDLE_DATA,
	ID_FIRST_CONNECTION,
	ID_SECOND_CONNECTION,
	ID_RECIEVE_GAME_INFO,
	ID_RECIEVE_BALL_INFO
};

Server::Server()
{
	init("200");
}

Server::Server(const char* serverPort)
{
	init(serverPort);
}

Server::~Server()
{
	cleanup();
}

void Server::init(const char* serverPort)
{
	mpServer = RakNet::RakPeerInterface::GetInstance();

	mpServer->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
	mpServer->SetIncomingPassword("hello", (int)strlen("hello"));

	RakNet::SocketDescriptor socketDescriptors[2];
	socketDescriptors[0].port = atoi(serverPort);
	socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
	socketDescriptors[1].port = atoi(serverPort);
	socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
	
	bool b = mpServer->Startup(16, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
	mpServer->SetMaximumIncomingConnections(16);

	if (!b)
	{
		puts("Server failed to start.  Terminating.");
		exit(1);
	}
	mpServer->SetOccasionalPing(true);
	mpServer->SetUnreliableTimeout(1000);

	printf("\nIP address for client to connect:\n");
	for (unsigned int i = 0; i < mpServer->GetNumberOfAddresses(); i++)
	{
		RakNet::SystemAddress sa = mpServer->GetInternalID(RakNet::UNASSIGNED_SYSTEM_ADDRESS, i);
		printf("%s\n", sa.ToString(false));
	}

	for (unsigned int i = 0; i < mClientPairs.size(); i++)
	{
		mClientPairs[i][0] = "";
		mClientPairs[i][1] = "";
	}

	initializeGameInfos();

	mNumGames = 0;

	mpTimer = new Timer();
	
	for (unsigned int i = 0; i < mGameInfos.size(); i++)
	{
		mGameInfos[i].ball.x = 400, mGameInfos[i].ball.y = 400;
		mGameInfos[i].ball.xVel = -30, mGameInfos[i].ball.yVel = 0;

		//mGameInfos[i].lPlayer.width = 20, mGameInfos[i].lPlayer.height = 100;
		//mGameInfos[i].rPlayer.width = 20, mGameInfos[i].rPlayer.height = 100;
	}
}


void Server::cleanup()
{
	mpServer->Shutdown(300);
	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(mpServer);
}

unsigned char Server::GetPacketIdentifier(RakNet::Packet *p)
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

void Server::update()
{
	//get packets from clients
	getPackets();

	updateGames();

	//if enough time has passed (30fps), broadcast game states to clients
	if (mpTimer->shouldUpdate())
	{
		broadcastGameInfo();
	}
}

void Server::updateGames()
{
	for (unsigned int i = 0; i < mGameInfos.size(); i++)
	{
		mGameInfos[i].ball.x += mGameInfos[i].ball.xVel;
		mGameInfos[i].ball.y += mGameInfos[i].ball.yVel;

		float p1LeftX = mGameInfos[i].lPlayer.x;
		float p1RightX = mGameInfos[i].lPlayer.x + 20;
		float p1TopY = mGameInfos[i].lPlayer.y;
		float p1BottomY = mGameInfos[i].lPlayer.y + 100;

		float p2LeftX = mGameInfos[i].rPlayer.x;
		float p2RightX = mGameInfos[i].rPlayer.x + 20;
		float p2TopY = mGameInfos[i].rPlayer.y;
		float p2BottomY = mGameInfos[i].rPlayer.y + 100;

		if (mGameInfos[i].ball.x <= p1RightX && mGameInfos[i].ball.x > p1LeftX && mGameInfos[i].ball.y > p1TopY  && mGameInfos[i].ball.y < p1BottomY)
		{
			//mGameInfos[i].ball.xVel *= -1;

			float paddleMidY = mGameInfos[i].lPlayer.y + 100 / 2;
			float ballMidY = mGameInfos[i].ball.y + 10;

			float angle = static_cast<float>((PI / 2) * (abs(paddleMidY - ballMidY) / (100 / 2)));
			mGameInfos[i].ball.xVel = static_cast<float>(sin(angle) + 5.0);
			mGameInfos[i].ball.yVel = static_cast<float>(cos(angle) + 5.0);
		}

		if ((mGameInfos[i].ball.x + 20) >= p2LeftX && (mGameInfos[i].ball.x + 20) < p2RightX && mGameInfos[i].ball.y > p2TopY  && mGameInfos[i].ball.y < p2BottomY)
		{
			//mGameInfos[i].ball.xVel *= -1;

			float paddleMidY = mGameInfos[i].rPlayer.y + 100 / 2;
			float ballMidY = mGameInfos[i].ball.y + 10;

			float angle = static_cast<float>((PI / 2) * (abs(paddleMidY - ballMidY) / (100 / 2)));
			mGameInfos[i].ball.xVel = static_cast<float>(sin(angle) + 5.0);
			mGameInfos[i].ball.yVel = static_cast<float>(cos(angle) + 5.0);
		}

		if (mGameInfos[i].ball.x < 0) mGameInfos[i].ball.xVel *= -1;
		if (mGameInfos[i].ball.x >= 1024 - 20) mGameInfos[i].ball.xVel *= -1;
		if (mGameInfos[i].ball.y < 0) mGameInfos[i].ball.yVel *= -1;
		if (mGameInfos[i].ball.y >= 768 - 20) mGameInfos[i].ball.yVel *= -1;
	}
}

void Server::getPackets()
{
	for (p = mpServer->Receive(); p; mpServer->DeallocatePacket(p), p = mpServer->Receive())
	{
		// We got a packet, get the identifier with our handy function
		packetIdentifier = GetPacketIdentifier(p);

		// Check if this is a network message packet
		switch (packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally
			printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));;
			break;
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;
		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", p->systemAddress.ToString(true));
			break;
		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));
			break;



		/*###########################################USER CHANGED/DEFINED###############################################*/


		case ID_NEW_INCOMING_CONNECTION:
		{
			// Somebody connected.  We have their IP now
			printf("New connection from %s\n", p->systemAddress.ToString(true));

			//if its the first client in a pair
			if (mClientPairs[mNumGames][0] == "" && mClientPairs[mNumGames][1] == "")
			{
				mClientPairs[mNumGames][0] = p->systemAddress;

				//make it the first player
				int id = ID_FIRST_CONNECTION;
				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}

			//if its the second client in a pair
			else
			{
				mClientPairs[mNumGames][1] = p->systemAddress;

				//set game info for this game
				mGameInfos[mNumGames].mID = ID_RECIEVE_GAME_INFO;

				//increase number of games
				mNumGames++;

				//set as second player
				int id = ID_SECOND_CONNECTION;
				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}

			break;
		}

		//the client sends over its paddle data
		case ID_SEND_PADDLE_DATA:
		{
			PaddleData paddle = *reinterpret_cast<PaddleData*>(p->data);

			//find correct game and client
			int j = 0;
			for (unsigned int i = 0; i < mClientPairs.size(); i++)
			{
				if (mClientPairs[i][0] == p->systemAddress)
				{
					mGameInfos[j].lPlayer.x = paddle.x;
					mGameInfos[j].lPlayer.y = paddle.y;
				}
				else if (mClientPairs[i][1] == p->systemAddress)
				{
					mGameInfos[j].rPlayer.x = paddle.x;
					mGameInfos[j].rPlayer.y = paddle.y;
				}
				if (i % 2 == 1) j++;
			}

			break;
		}
		default:
			//try to print data
			printf("%s\n", p->data);
			break;
		}
	}
}

void Server::broadcastGameInfo()
{
	//send each gameinfo to the correct clients
	int j = 0;
	for (unsigned int i = 0; i < mClientPairs.size(); i++)
	{
		GameInfo info = mGameInfos[j];
		info.mID = ID_RECIEVE_GAME_INFO;
		mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][0], false);
		mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][1], false);
		if (i % 2 == 1) j++;
	}
}

void Server::initializeGameInfos()
{
	int j = 0;
	for (unsigned int i = 0; i < mClientPairs.size(); i++)
	{
		mGameInfos[j].mID = ID_RECIEVE_GAME_INFO;
		
		mGameInfos[j].ball.x = 400;
		mGameInfos[j].ball.y = 400;
		mGameInfos[j].ball.xVel = -30;
		mGameInfos[j].ball.yVel = 0;

		mGameInfos[j].lPlayer.goalsScored = 0;
		mGameInfos[j].lPlayer.x = 200;
		mGameInfos[j].lPlayer.y = 768 / 2;

		mGameInfos[j].rPlayer.goalsScored = 0;
		mGameInfos[j].rPlayer.x = 1024 - 200 - 20;
		mGameInfos[j].rPlayer.y = 768 / 2;

		if (i % 2 == 1) j++;
	}
}