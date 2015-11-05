#include "Server.h"

enum MessageTypes
{
	// For the user to use.  Start your first enumeration at this value.
	//ID_USER_PACKET_ENUM,
	//-------------------------------------------------------------------------------------------------------------

	ID_SEND_SHAPE = ID_USER_PACKET_ENUM,
	ID_RECEIVE_SHAPE,
	ID_FIRST_CONNECTION,
	ID_SECOND_CONNECTION
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
	
	bool b = mpServer->Startup(6, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
	mpServer->SetMaximumIncomingConnections(6);

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

	mNumGames = 0;
	QueryPerformanceFrequency(&mFrequency);
	QueryPerformanceCounter(&mStartTime);
}


void Server::cleanup()
{
	mpServer->Shutdown(300);
	// We're done with the network
	RakNet::RakPeerInterface::DestroyInstance(mpServer);
}

double Server::calcDifferenceInMS(LARGE_INTEGER from, LARGE_INTEGER to) const
{
	double difference = (double)(to.QuadPart - from.QuadPart) / (double)(mFrequency.QuadPart);
	return difference * 1000;
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
	// This sleep keeps RakNet responsive
	RakSleep(30);

	// Get a packet from either the server or the client
	getPackets();

	QueryPerformanceCounter(&mEndTime);
	if (calcDifferenceInMS(mStartTime, mEndTime) >= (1000.0/30.0))
	{
		broadcastGameInfo();
		QueryPerformanceCounter(&mStartTime);
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

			if (mClientPairs[mNumGames][0] == "" && mClientPairs[mNumGames][1] == "")
			{
				mClientPairs[mNumGames][0] = p->systemAddress;

				int id = ID_FIRST_CONNECTION;

				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}
			else
			{
				mClientPairs[mNumGames][1] = p->systemAddress;

				//add gameinfo struct here

				mNumGames++;

				int id = ID_SECOND_CONNECTION;
				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}

			break;
		}
		case ID_SEND_SHAPE:
		{
			ShapePosition pos = *reinterpret_cast<ShapePosition*>(p->data);
			pos.mID = ID_RECEIVE_SHAPE;
			for (unsigned int i = 0; i < mClientPairs.size(); i++)
			{
				if (mClientPairs[i][0] == p->systemAddress)
				{
					mpServer->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][1], false);
				}
				else
				{
					mpServer->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][0], false);
				}
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
	for (unsigned int i = 0; i < mClientPairs.size; i++)
	{
		mGameInfos[i];
		mpServer->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][0], false);
		mpServer->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mClientPairs[i][1], false);
	}
}