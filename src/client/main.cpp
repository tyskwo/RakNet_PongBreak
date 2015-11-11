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
	sf::RenderWindow window(sf::VideoMode(1024, 768), "PONGBREAK");
	window.setFramerateLimit(60);





//##############################################################INIT CLIENT############################################
	//connect to a random port
	srand(time(NULL));
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

	if (mpClient->getFirstConnected())
	{
		player.setFillColor(sf::Color(200, 10, 10));
		player.setPosition(sf::Vector2f(200.0, 0.0));

		opponent.setFillColor(sf::Color(10, 10, 200));
		opponent.setPosition(sf::Vector2f(1024.0 - 200.0 - 20.0, 0.0));
	}
	else
	{
		player.setFillColor(sf::Color(10, 10, 200));
		player.setPosition(sf::Vector2f(1024.0 - 200.0 - 20.0, 0.0));

		opponent.setFillColor(sf::Color(200, 10, 10));
		opponent.setPosition(sf::Vector2f(200.0, 0.0));
	}

	ball.setFillColor(sf::Color::Cyan);
	ball.setPosition(400, 400);

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
		/*ObjectInfo info = mpClient->getOpponentInterpolation().GetNext(mpClient->getElapsedT());
		if (mpClient->getFirstConnected())
		{
			opponent.setPosition(sf::Vector2f(mpClient->getGameInfo().rPlayer.x, info.GetState().mY));
		}
		else
		{
			opponent.setPosition(sf::Vector2f(mpClient->getGameInfo().lPlayer.x, info.GetState().mY));
		}*/

		ObjectInfo binfo = mpClient->getBallInterpolation().GetNext(mpClient->getElapsedT());
		ball.setPosition(binfo.GetState().mX, binfo.GetState().mY);





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
		window.draw(player);
		window.draw(opponent);
		window.draw(ball);

		window.display();
	}

	return 0;
}