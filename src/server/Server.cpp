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
	ID_RECIEVE_BALL_INFO,
	ID_START_GAME
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
	for (unsigned int i = 0; i < mVelocityMultipliers.size(); i++)
	{
		mVelocityMultipliers[i] = 5.0f;
	}

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
		mClientPairs[i][0] = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
		mClientPairs[i][1] = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
	}

	for (unsigned int i = 0; i < mGuidPairs.size(); i++)
	{
		mGuidPairs[i][0] = RakNet::UNASSIGNED_RAKNET_GUID;
		mGuidPairs[i][1] = RakNet::UNASSIGNED_RAKNET_GUID;
	}

	initializeGameInfos();

	mNumGames = 0;

	mpTimer = new Timer();
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

bool Server::doesCollide(const Rectangle& rect1, const Rectangle& rect2)
{
	bool horizontalCollision = false;
	bool verticalCollision = false;

	if ((rect1.leftX < rect2.rightX && rect1.leftX > rect2.leftX) || (rect1.rightX < rect2.rightX && rect1.rightX > rect2.leftX))
	{
		horizontalCollision = true;
	}

	if ((rect1.bottomY > rect2.topY && rect1.bottomY < rect2.bottomY) || (rect1.topY < rect2.bottomY && rect1.topY > rect2.topY))
	{
		verticalCollision = true;
	}

	return horizontalCollision && verticalCollision;
}

void Server::updateGames()
{
	Rectangle ballRect;
	ballRect.width = 20, ballRect.height = 20;
	Rectangle p1PaddleRect;
	p1PaddleRect.width = 20, p1PaddleRect.height = 100;
	Rectangle p2PaddleRect;
	p2PaddleRect.width = 20, p2PaddleRect.height = 100;

	bool stillColliding = false;


	for (unsigned int i = 0; i < mGameInfos.size(); i++)
	{
		float xVel = mGameInfos[i].ball.xVel;
		float yVel = mGameInfos[i].ball.yVel;

		if (mGameInfos[i].started)
		{
			mGameInfos[i].ball.x += mGameInfos[i].ball.xVel;
			mGameInfos[i].ball.y += mGameInfos[i].ball.yVel;

			
			ballRect.leftX   = mGameInfos[i].ball.x;
			ballRect.rightX  = mGameInfos[i].ball.x + ballRect.width;
			ballRect.topY    = mGameInfos[i].ball.y;
			ballRect.bottomY = mGameInfos[i].ball.y + ballRect.height;

			
			p1PaddleRect.leftX   = mGameInfos[i].lPlayer.x;
			p1PaddleRect.rightX  = mGameInfos[i].lPlayer.x + p1PaddleRect.width;
			p1PaddleRect.topY    = mGameInfos[i].lPlayer.y;
			p1PaddleRect.bottomY = mGameInfos[i].lPlayer.y + p1PaddleRect.height;

			
			p2PaddleRect.leftX   = mGameInfos[i].rPlayer.x;
			p2PaddleRect.rightX  = mGameInfos[i].rPlayer.x + p2PaddleRect.width;
			p2PaddleRect.topY    = mGameInfos[i].rPlayer.y;
			p2PaddleRect.bottomY = mGameInfos[i].rPlayer.y + p2PaddleRect.height;




			if (doesCollide(ballRect, p1PaddleRect))
			{
				mVelocityMultipliers[i] += 0.2f;

				if (mGameInfos[i].ball.collided)
				{
					stillColliding = true;
				}
				else
				{
					float direction = mGameInfos[i].ball.xVel;

					float paddleMidY = mGameInfos[i].lPlayer.y + 100 / 2;
					float ballMidY = mGameInfos[i].ball.y + 10;

					float angle = static_cast<float>((3.0f * PI / 12.0f) * ((paddleMidY - ballMidY) / (75.0f / 2.0f)));
					if (direction > 0)
					{
						mGameInfos[i].ball.xVel = static_cast<float>(-cos(angle) * mVelocityMultipliers[i]);
						mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
					}
					else
					{
						mGameInfos[i].ball.xVel = static_cast<float>(cos(angle) * mVelocityMultipliers[i]);
						mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
					}

					mGameInfos[i].ball.collided = true;
				}
			}

			if (doesCollide(ballRect, p2PaddleRect))
			{
				mVelocityMultipliers[i] += 0.2f;

				if (mGameInfos[i].ball.collided)
				{
					stillColliding = true;
				}
				else
				{
					float direction = mGameInfos[i].ball.xVel;

					float paddleMidY = mGameInfos[i].rPlayer.y + 100 / 2;
					float ballMidY = mGameInfos[i].ball.y + 10;

					float angle = static_cast<float>((3.0f * PI / 12.0f) * ((paddleMidY - ballMidY) / (75.0f / 2.0f)));
					if (direction > 0)
					{
						mGameInfos[i].ball.xVel = static_cast<float>(-cos(angle) * mVelocityMultipliers[i]);
						mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
					}
					else
					{
						mGameInfos[i].ball.xVel = static_cast<float>(cos(angle) * mVelocityMultipliers[i]);
						mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
					}

					mGameInfos[i].ball.collided = true;
				}
			}




			for (unsigned int j = 0; j < mGameInfos[i].lPlayer.brickLocs.size(); j++)
			{
				if (mGameInfos[i].lPlayer.bricks[j % 6][j / 6] == true)
				{
					Rectangle brickRect;
					brickRect.width = 20, brickRect.height = 75;
					brickRect.leftX = mGameInfos[i].lPlayer.brickLocs[j].x;
					brickRect.rightX = brickRect.leftX + brickRect.width;
					brickRect.topY = mGameInfos[i].lPlayer.brickLocs[j].y;
					brickRect.bottomY = brickRect.topY + brickRect.height;

					if (doesCollide(ballRect, brickRect))
					{
						mVelocityMultipliers[i] += 0.2f;

						if (mGameInfos[i].ball.collided)
						{
							stillColliding = true;
						}
						else
						{
							mGameInfos[i].rPlayer.goalsScored++;
							mGameInfos[i].lPlayer.bricks[j % 6][j / 6] = false;

							float direction = mGameInfos[i].ball.xVel;

							float brickMidY = brickRect.topY + brickRect.height / 2;
							float ballMidY = mGameInfos[i].ball.y + 10;

							float angle = static_cast<float>((3.0f * PI / 12.0f) * ((brickMidY - ballMidY) / (75.0f / 2.0f)));
							if (direction > 0)
							{
								mGameInfos[i].ball.xVel = static_cast<float>(-cos(angle) * mVelocityMultipliers[i]);
								mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
							}
							else
							{
								mGameInfos[i].ball.xVel = static_cast<float>(cos(angle) * mVelocityMultipliers[i]);
								mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
							}

							mGameInfos[i].ball.collided = true;
						}
					}
				}
			}




			for (unsigned int j = 0; j < mGameInfos[i].rPlayer.brickLocs.size(); j++)
			{
				if (mGameInfos[i].rPlayer.bricks[j % 6][j / 6] == true)
				{
					Rectangle brickRect;
					brickRect.width = 20, brickRect.height = 75;
					brickRect.leftX = mGameInfos[i].rPlayer.brickLocs[j].x;
					brickRect.rightX = brickRect.leftX + brickRect.width;
					brickRect.topY = mGameInfos[i].rPlayer.brickLocs[j].y;
					brickRect.bottomY = brickRect.topY + brickRect.height;

					if (doesCollide(ballRect, brickRect))
					{
						mVelocityMultipliers[i] += 0.2f;

						if (mGameInfos[i].ball.collided)
						{
							stillColliding = true;
						}
						else
						{
							mGameInfos[i].lPlayer.goalsScored++;
							mGameInfos[i].rPlayer.bricks[j % 6][j / 6] = false;

							float direction = mGameInfos[i].ball.xVel;

							float brickMidY = brickRect.topY + brickRect.height / 2;
							float ballMidY = mGameInfos[i].ball.y + 10;

							float angle = static_cast<float>((3.0f * PI / 12.0f) * ((brickMidY - ballMidY) / (75.0f / 2.0f)));
							if (direction > 0)
							{
								mGameInfos[i].ball.xVel = static_cast<float>(-cos(angle) * mVelocityMultipliers[i]);
								mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
							}
							else
							{
								mGameInfos[i].ball.xVel = static_cast<float>(cos(angle) * mVelocityMultipliers[i]);
								mGameInfos[i].ball.yVel = static_cast<float>(-sin(angle) * mVelocityMultipliers[i]);
							}

							mGameInfos[i].ball.collided = true;
						}
					}
				}
			}




			if (mGameInfos[i].ball.x < 0)
			{
				mVelocityMultipliers[i] += 0.05f;
				mGameInfos[i].ball.xVel *= -1;
				mGameInfos[i].rPlayer.goalsScored += 3;
				mGameInfos[i].ball.collided = true;
			}
			if (mGameInfos[i].ball.x >= 1024 - 20)
			{
				mVelocityMultipliers[i] += 0.05f;
				mGameInfos[i].ball.xVel *= -1;
				mGameInfos[i].lPlayer.goalsScored += 3;
				mGameInfos[i].ball.collided = true;
			}
			if (mGameInfos[i].ball.y < 0)
			{
				mVelocityMultipliers[i] += 0.05f;
				mGameInfos[i].ball.yVel *= -1;
				mGameInfos[i].ball.collided = true;
			}
			if (mGameInfos[i].ball.y >= 768 - 20)
			{
				mVelocityMultipliers[i] += 0.05f;
				mGameInfos[i].ball.yVel *= -1;
				mGameInfos[i].ball.collided = true;
			}

			if (mVelocityMultipliers[i] >= 12.0f) mVelocityMultipliers[i] = 12.0f;

			if (!stillColliding)
			{
				mGameInfos[i].ball.collided = false;
			}






			bool allBricksGone = true;
			for (unsigned int m = 0; m < mGameInfos[i].lPlayer.bricks.size(); m++)
			{
				if (mGameInfos[i].lPlayer.bricks[m % 6][m / 6])
				{
					allBricksGone = false;
				}
			}

			for (unsigned int n = 0; n < mGameInfos[i].lPlayer.bricks.size(); n++)
			{
				if (mGameInfos[i].lPlayer.bricks[n % 6][n / 6])
				{
					allBricksGone = false;
				}
			}
			if (allBricksGone || mGameInfos[i].lPlayer.goalsScored >= 50 || mGameInfos[i].rPlayer.goalsScored >= 50)
			{
				mGameInfos[i].finished = true;
				mGameInfos[i].started = false;
			}
		}
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
			if (mClientPairs[mNumGames][0] == RakNet::UNASSIGNED_SYSTEM_ADDRESS && mClientPairs[mNumGames][1] == RakNet::UNASSIGNED_SYSTEM_ADDRESS)
			{
				mClientPairs[mNumGames][0] = p->systemAddress;
				mGuidPairs[mNumGames][0] = p->guid;

				GameInfo info = mGameInfos[mNumGames];
				info.mID = ID_FIRST_CONNECTION;
				mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->guid, false);
			}

			//if its the second client in a pair
			else
			{
				mClientPairs[mNumGames][1] = p->systemAddress;
				mGuidPairs[mNumGames][1] = p->guid;

				GameInfo info = mGameInfos[mNumGames];
				info.mID = ID_SECOND_CONNECTION;
				mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->guid, false);

				//increase number of games
				mNumGames++;
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
				if (mGuidPairs[i][0] == p->guid)
				{
					mGameInfos[j].lPlayer.x = paddle.x;
					mGameInfos[j].lPlayer.y = paddle.y;
				}
				else if (mGuidPairs[i][1] == p->guid)
				{
					mGameInfos[j].rPlayer.x = paddle.x;
					mGameInfos[j].rPlayer.y = paddle.y;
				}
				if (i % 2 == 1) j++;
			}

			break;
		}
		//the client sends over time to start game
		case ID_START_GAME:
		{
			//find correct game and client
			int j = 0;
			for (unsigned int i = 0; i < mClientPairs.size(); i++)
			{
				if (mGuidPairs[i][0] == p->guid || mGuidPairs[i][1] == p->guid)
				{
					resetGame(j);
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
		std::cout << mGuidPairs[i][0].ToString() << " " << mGuidPairs[i][1].ToString() << std::endl;

		GameInfo info = mGameInfos[j];
		info.mID = ID_RECIEVE_GAME_INFO;
		mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mGuidPairs[i][0], false);
		mpServer->Send((const char*)&info, sizeof(info), HIGH_PRIORITY, RELIABLE_ORDERED, 0, mGuidPairs[i][1], false);
		if (i % 2 == 1) j++;
	}

	std::cout << std::endl;
}

void Server::initializeGameInfos()
{
	int j = 0;
	for (unsigned int i = 0; i < mClientPairs.size(); i++)
	{
		mGameInfos[j].mID = ID_RECIEVE_GAME_INFO;
		
		mGameInfos[j].ball.x = HALF_SCREEN_WIDTH - 10;
		mGameInfos[j].ball.y = HALF_SCREEN_HEIGHT - 10;
		mGameInfos[j].ball.xVel = 7;
		mGameInfos[j].ball.yVel = 0;
		mGameInfos[j].ball.collided = false;
		mVelocityMultipliers[j] = 5.0f;

		mGameInfos[j].lPlayer.goalsScored = 0;
		mGameInfos[j].lPlayer.x = 200.0f;
		mGameInfos[j].lPlayer.y = HALF_SCREEN_HEIGHT - 50.0f;
		mGameInfos[j].lPlayer.velocity = 0.0f;

		mGameInfos[j].rPlayer.goalsScored = 0;
		mGameInfos[j].rPlayer.x = SCREEN_WIDTH - 220.0f;
		mGameInfos[j].rPlayer.y = HALF_SCREEN_HEIGHT - 50.0f;
		mGameInfos[j].rPlayer.velocity = 0.0f;

		for (unsigned int k = 0; k < mGameInfos[j].lPlayer.brickLocs.size(); k++)
		{
			mGameInfos[j].lPlayer.brickLocs[k].x = 10.0f + 40.0f * (k / 6);
			mGameInfos[j].lPlayer.brickLocs[k].y = (HALF_SCREEN_HEIGHT + 25.0f - 125.0f * 3.0f) + 125.0f * (k % 6);
		}

		for (unsigned int l = 0; l < mGameInfos[j].rPlayer.brickLocs.size(); l++)
		{
			mGameInfos[j].rPlayer.brickLocs[l].x = SCREEN_WIDTH - 10.0f - 20.0f - 40.0f * (l / 6);
			mGameInfos[j].rPlayer.brickLocs[l].y = (HALF_SCREEN_HEIGHT + 25.0f - 125.0f * 3.0f) + 125.0f * (l % 6);
		}

		for (unsigned int l = 0; l < mGameInfos[j].lPlayer.brickLocs.size(); l++)
		{
			mGameInfos[j].lPlayer.bricks[l % 6][l / 6] = true;
		}

		for (unsigned int l = 0; l < mGameInfos[j].rPlayer.brickLocs.size(); l++)
		{
			mGameInfos[j].rPlayer.bricks[l % 6][l / 6] = true;
		}

		if (i % 2 == 1) j++;
	}
}

void Server::resetGame(int index)
{
	mVelocityMultipliers[index] = 5.0f;

	mGameInfos[index].mID = ID_RECIEVE_GAME_INFO;

	mGameInfos[index].started = true;
	mGameInfos[index].finished = false;

	mGameInfos[index].ball.x = HALF_SCREEN_WIDTH - 10;
	mGameInfos[index].ball.y = HALF_SCREEN_HEIGHT - 10;
	mGameInfos[index].ball.xVel = 7;
	mGameInfos[index].ball.yVel = 0;
	mGameInfos[index].ball.collided = false;

	mGameInfos[index].lPlayer.goalsScored = 0;
	mGameInfos[index].lPlayer.x = 200.0f;
	mGameInfos[index].lPlayer.y = HALF_SCREEN_HEIGHT - 50.0f;
	mGameInfos[index].lPlayer.velocity = 0.0f;

	mGameInfos[index].rPlayer.goalsScored = 0;
	mGameInfos[index].rPlayer.x = SCREEN_WIDTH - 220.0f;
	mGameInfos[index].rPlayer.y = HALF_SCREEN_HEIGHT - 50.0f;
	mGameInfos[index].rPlayer.velocity = 0.0f;

	for (unsigned int k = 0; k < mGameInfos[index].lPlayer.brickLocs.size(); k++)
	{
		mGameInfos[index].lPlayer.brickLocs[k].x = 10.0f + 40.0f * (k / 6);
		mGameInfos[index].lPlayer.brickLocs[k].y = (HALF_SCREEN_HEIGHT + 25.0f - 125.0f * 3.0f) + 125.0f * (k % 6);
	}

	for (unsigned int l = 0; l < mGameInfos[index].rPlayer.brickLocs.size(); l++)
	{
		mGameInfos[index].rPlayer.brickLocs[l].x = SCREEN_WIDTH - 10.0f - 20.0f - 40.0f * (l / 6);
		mGameInfos[index].rPlayer.brickLocs[l].y = (HALF_SCREEN_HEIGHT + 25.0f - 125.0f * 3.0f) + 125.0f * (l % 6);
	}

	for (unsigned int l = 0; l < mGameInfos[index].lPlayer.brickLocs.size(); l++)
	{
		mGameInfos[index].lPlayer.bricks[l % 6][l / 6] = true;
	}

	for (unsigned int l = 0; l < mGameInfos[index].rPlayer.brickLocs.size(); l++)
	{
		mGameInfos[index].rPlayer.bricks[l % 6][l / 6] = true;
	}
}