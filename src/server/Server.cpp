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

	for (int i = 0; i < mClientPairs.size(); i++)
	{
		mClientPairs[i][0] = -1;
		mClientPairs[i][1] = -1;
	}

	mNumGames = 0;
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
	// This sleep keeps RakNet responsive
	RakSleep(30);

	// Get a packet from either the server or the client
	getPackets();
	sendPacket();
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


		case ID_NEW_INCOMING_CONNECTION:
			// Somebody connected.  We have their IP now
			printf("ID_NEW_INCOMING_CONNECTION from %s with GUID %s\n", p->systemAddress.ToString(true), p->guid.ToString());
			clientID = p->systemAddress; // Record the player ID of the client

			printf("Remote internal IDs:\n");
			for (int index = 0; index < MAXIMUM_NUMBER_OF_INTERNAL_IDS; index++)
			{
				RakNet::SystemAddress internalId = mpServer->GetInternalID(p->systemAddress, index);
				if (internalId != RakNet::UNASSIGNED_SYSTEM_ADDRESS)
				{
					printf("%i. %s\n", index + 1, internalId.ToString(true));
				}
			}

			if (mClientPairs[mNumGames][0] == -1 && mClientPairs[mNumGames][1] == -1)
			{
				std::cout << (int)p->systemAddress.GetPort();
				mClientPairs[mNumGames][0] = (int)p->systemAddress.GetPort();

				int id = ID_FIRST_CONNECTION;
				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}
			else
			{
				mClientPairs[mNumGames][1] = (int)p->systemAddress.GetPort();
				mNumGames++;

				int id = ID_SECOND_CONNECTION;
				mpServer->Send((const char*)&id, sizeof(id), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
			}

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

		case ID_SEND_SHAPE:
		{
			ShapePosition pos = *reinterpret_cast<ShapePosition*>(p->data);
			pos.mID = ID_RECEIVE_SHAPE;

			mpServer->Send((const char*)&pos, sizeof(pos), HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			break;
		}
		default:
			// The server knows the static data of all clients, so we can prefix the message
			// With the name data
			printf("%s\n", p->data);

			// Relay the message.  We prefix the name for other clients.  This demonstrates
			// That messages can be changed on the server before being broadcast
			// Sending is the same as before
			sprintf_s(mMessage, "%s", p->data);
			mpServer->Send(mMessage, (const int)strlen(mMessage) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);

			break;
		}
	}
}

void Server::sendPacket()
{
	if (kbhit())
	{
		Gets(mMessage, sizeof(mMessage));

		if (strcmp(mMessage, "quit") == 0)
		{
			puts("Quitting.");
			cleanup();
		}

		mpServer->Send(mMessage, (int)strlen(mMessage) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}
}