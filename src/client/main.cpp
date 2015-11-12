#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include "Client.h"
#include "ObjectInfo.h"

int main()
{
	// create the window
	const float SCREEN_WIDTH = 1024.0;
	const float SCREEN_HEIGHT = 768.0;
	const float HALF_SCREEN_WIDTH = SCREEN_WIDTH / 2;
	const float HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;

	sf::RenderWindow window(sf::VideoMode(1024, 768), "PONGBREAK");
	window.setFramerateLimit(60);




//##############################################################INIT CLIENT############################################
	//connect to a random port
	srand(static_cast<unsigned int>(time(NULL)));
	int randPortNumber = rand() % (301 - 201 + 1) + 201;
	std::stringstream converter;
	converter << randPortNumber;
	std::string temp_str = converter.str();
	char* char_type = (char*)temp_str.c_str();

	//connect to external ip
	std::string ipAddress = "";
	std::cout << "Input server IP or \"localhost\": ";
	std::cin >> ipAddress;
	Client* mpClient = new Client(char_type, ipAddress.c_str(), "200");

	//while trying to connect, don't update the game logic
	while (!mpClient->getConnected()) { mpClient->update(); }





//########################################################INIT SHAPES##################################################
	sf::RectangleShape player(sf::Vector2f(20, 100));
	sf::RectangleShape opponent(sf::Vector2f(20, 100));
	sf::RectangleShape ball(sf::Vector2f(20, 20));

	std::array<sf::RectangleShape, 18> playerBricks;
	std::array<sf::RectangleShape, 18> opponentBricks;

	sf::Text playerScore;
	sf::Text opponentScore;
	std::stringstream playerScoreStream;
	std::stringstream opponentScoreStream;

	//playerScore.setFont(sf::Font::)

	if (mpClient->getFirstConnected())
	{
		player.setFillColor(sf::Color(200, 10, 10));
		player.setPosition(sf::Vector2f(200.0, 0.0));

		opponent.setFillColor(sf::Color(10, 10, 200));
		opponent.setPosition(sf::Vector2f(SCREEN_WIDTH - 200.0f - 20.0f, 0.0f));

		for (unsigned int i = 0; i < playerBricks.size(); i++)
		{
			playerBricks[i] = sf::RectangleShape(sf::Vector2f(20, 75));
			playerBricks[i].setPosition(10.0f + 40.0f * (i / 6), (HALF_SCREEN_HEIGHT - 100.0f * 3.0f) + 100.0f * (i % 6));
		}

		for (unsigned int i = 0; i < opponentBricks.size(); i++)
		{
			opponentBricks[i] = sf::RectangleShape(sf::Vector2f(20, 75));
			opponentBricks[i].setPosition(SCREEN_WIDTH - 10.0f - 20.0f - 40.0f * (i / 6), (HALF_SCREEN_HEIGHT - 100.0f * 3.0f) + 100.0f * (i % 6));
		}

		playerScore.setString("0");
		playerScore.setPosition(50.0f, 25.0f);
		opponentScore.setString("0");
		opponentScore.setPosition(SCREEN_WIDTH - 50.0f, 25.0f);
	}
	else
	{
		player.setFillColor(sf::Color(10, 10, 200));
		player.setPosition(sf::Vector2f(SCREEN_WIDTH - 200.0f - 20.0f, 0.0f));

		opponent.setFillColor(sf::Color(200, 10, 10));
		opponent.setPosition(sf::Vector2f(200.0, 0.0));

		for (unsigned int i = 0; i < opponentBricks.size(); i++)
		{
			opponentBricks[i] = sf::RectangleShape(sf::Vector2f(20, 75));
			opponentBricks[i].setPosition(10.0f + 40.0f * (i / 6), (HALF_SCREEN_HEIGHT - 100.0f * 3.0f) + 100.0f * (i % 6));
		}

		for (unsigned int i = 0; i < playerBricks.size(); i++)
		{
			playerBricks[i] = sf::RectangleShape(sf::Vector2f(20, 75));
			playerBricks[i].setPosition(SCREEN_WIDTH - 10.0f - 20.0f - 40.0f * (i / 6), (HALF_SCREEN_HEIGHT - 100.0f * 3) + 100.0f * (i % 6));
		}

		playerScore.setString("0");
		playerScore.setPosition(SCREEN_WIDTH - 50.0f, 25.0f);
		opponentScore.setString("0");
		opponentScore.setPosition(50.0f, 25.0f);
	}

	ball.setFillColor(sf::Color::Cyan);
	ball.setPosition(HALF_SCREEN_WIDTH, HALF_SCREEN_HEIGHT);

	float yPos = 0.0;



//####################################################RUN PROGRAM#####################################################
	while (window.isOpen())
	{
		mpClient->update();

//#############################################GET INPUT##############################################################
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			yPos -= 5.0;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			yPos += 5.0;
		}
		player.setPosition(player.getPosition().x, yPos);
		mpClient->setPaddleLoc(player.getPosition().x, yPos);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !mpClient->getFirstConnected())
		{
			mpClient->setGameStart();
		}

//##############################################INTERPOLATE###########################################################
		ObjectInfo info = mpClient->getOpponentInterpolation().GetNext(mpClient->getElapsedT());
		if (mpClient->getFirstConnected())
		{
			opponent.setPosition(sf::Vector2f(mpClient->getGameInfo().rPlayer.x, info.GetState().mY));
		}
		else
		{
			opponent.setPosition(sf::Vector2f(mpClient->getGameInfo().lPlayer.x, info.GetState().mY));
		}

		ObjectInfo binfo = mpClient->getBallInterpolation().GetNext(mpClient->getElapsedT());
		ball.setPosition(binfo.GetState().mX, binfo.GetState().mY);
		//ball.setPosition(mpClient->getGameInfo().ball.x, mpClient->getGameInfo().ball.y);


//##############################################UPDATE GOAL TEXT#####################################################
		//std::cout << mpClient->getGameInfo().lPlayer.goalsScored << " " << mpClient->getGameInfo().rPlayer.goalsScored << std::endl;
		if (mpClient->getFirstConnected())
		{
			playerScoreStream << mpClient->getGameInfo().lPlayer.goalsScored;
			opponentScoreStream << mpClient->getGameInfo().rPlayer.goalsScored;
		}
		else
		{
			playerScoreStream << mpClient->getGameInfo().rPlayer.goalsScored;
			opponentScoreStream << mpClient->getGameInfo().lPlayer.goalsScored;
		}

		playerScore.setString(playerScoreStream.str());
		opponentScore.setString(opponentScoreStream.str());



//###############################################WINDOW CLOSE#########################################################
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();
		}





//################################################CLEAR AND DRAW######################################################
		window.clear(sf::Color::Black);

		for (unsigned int i = 0; i < playerBricks.size(); i++)
		{
			if (mpClient->getFirstConnected())
			{
				if (mpClient->getGameInfo().lPlayer.bricks[i % 6][i / 6] == true)
				{
					window.draw(playerBricks[i]);
				}
			}
			else
			{
				if (mpClient->getGameInfo().rPlayer.bricks[i % 6][i / 6] == true)
				{
					window.draw(playerBricks[i]);
				}
			}
		}

		for (unsigned int i = 0; i < opponentBricks.size(); i++)
		{
			if (mpClient->getFirstConnected())
			{
				if (mpClient->getGameInfo().rPlayer.bricks[i % 6][i / 6] == true)
				{
					window.draw(opponentBricks[i]);
				}
			}
			else
			{
				if (mpClient->getGameInfo().lPlayer.bricks[i % 6][i / 6] == true)
				{
					window.draw(opponentBricks[i]);
				}
			}
		}

		window.draw(player);
		window.draw(opponent);
		window.draw(ball);
		window.draw(playerScore);
		window.draw(opponentScore);

		window.display();
	}

	return 0;
}