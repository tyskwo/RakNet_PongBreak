#ifndef CLIENT_H
#define CLIENT_H

#include "RakPeerInterface.h"
#include "../common/Timer.h"
#include "ObjectInfo.h"
#include <array>

#pragma pack(push, 1)
//struct for position values
struct Position
{
	unsigned char mID;
	float x, y;
};
#pragma pack(pop)

//#pragma pack(push, 1)
//struct for player values
struct Player
{
	float x, y;
	std::array<std::array<bool, 3>, 6> bricks;

	int goalsScored;
};
//#pragma pack(pop)

//#pragma pack(push, 1)
//struct for ball values
struct Ball
{
	float x, y;
	float xVel, yVel;
};
//#pragma pack(pop)

//struct to receive game state from server
#pragma pack(push, 1)
struct GameInfo
{
	unsigned char mID;

	bool started = false;

	Player lPlayer;
	Player rPlayer;
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
	void sendPaddleData(float x, float y);

	//send game start message
	void sendGameStart();


	//set flags for whether it was first connected, or connected second
	inline void setFirstConnected(bool wasFirst)		{ mWasFirstConnected = wasFirst; };
	inline bool getFirstConnected()			     { return mWasFirstConnected; };

	inline void setConnected(bool wasSecond)			{ mIsConnected = wasSecond; };
	inline bool getConnected()					 { return mIsConnected; };


	inline void setGameStart()	{ mGameInfo.started = true; mSendGameStart = true; };


	inline GameInfo getGameInfo() { return mGameInfo; };

	const ObjectInfoBuffer& getOpponentInterpolation() { return mOpponentInterpolation; };
	const ObjectInfoBuffer& getBallInterpolation()	   { return mBallInterpolation;     };

	const double& getDeltaT()   { return mpTimer->getDeltaT();  };
	const double& getElapsedT() { return mpTimer->getElapsedT(); };

	void setPaddleLoc(const float& x, const float& y);

private:
	//pointer to client object
	RakNet::RakPeerInterface* mpClient;

	//Holds packets
	RakNet::Packet* mpPacket;

	//socket descriptor
	RakNet::SocketDescriptor mSocketDescriptor;

	//get and identify packet
	unsigned char mPacketIdentifier;
	unsigned char GetPacketIdentifier(RakNet::Packet* pPacket);
	void getPackets();

	//values for the connection values.
	char mIPaddress[64], mServerPort[3], mClientPort[3];

	//flag for if client is connected, and if so, first player
	bool mIsConnected;
	bool mWasFirstConnected;

	//flag to send over game started packet
	bool mSendGameStart;

	//timer
	Timer* mpTimer;

	//game info
	GameInfo mGameInfo;

	//interpolators for opponent and ball
	ObjectInfoBuffer mOpponentInterpolation, mBallInterpolation;
};

#endif