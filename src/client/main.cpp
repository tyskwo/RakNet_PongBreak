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
	sf::RenderWindow window(sf::VideoMode(800, 600), "PONGBREAK");
	window.setFramerateLimit(60);

	//connect
	srand(time(NULL));
	int randPortNumber = rand() % (301 - 201 + 1) + 201;
	std::stringstream converter;
	converter << randPortNumber;
	std::string temp_str = converter.str();
	char* char_type = (char*)temp_str.c_str();

	std::string ipAddress = "";
	std::cout << "Input server IP or \"localhost\": ";
	std::cin >> ipAddress;
	Client* mpClient = new Client(char_type, ipAddress.c_str(), "200");

	//while trying to conncet, don't update the game logic
	while (!mpClient->getConnected()) { mpClient->update(); }


	//#######Everything below here should be elsewhere########

	sf::RectangleShape rect(sf::Vector2f(20, 100));
	sf::RectangleShape otherRect(sf::Vector2f(20, 100));
	sf::RectangleShape ball(sf::Vector2f(20, 20));

	if (mpClient->getFirstConnected())
	{
		rect.setFillColor(sf::Color(200, 10, 10));
		rect.setPosition(sf::Vector2f(100.0f, 0.0f));

		otherRect.setFillColor(sf::Color(10, 10, 200));
	}
	else
	{
		rect.setFillColor(sf::Color(10, 10, 200));
		rect.setPosition(sf::Vector2f(700.0f, 0.0f));

		otherRect.setFillColor(sf::Color(200, 10, 10));
	}

	ball.setFillColor(sf::Color::Cyan);

	sf::Vector2f rectY = rect.getPosition();

	float rectVelocity = 0.0f;

	sf::Vector2f prevPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);
	sf::Vector2f currPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);

	GameInfo currGameInfo;

	//##############################################################

	// run the program as long as the window is open
	while (window.isOpen())
	{
		mpClient->update();

		if (mpClient->getFirstConnected())
		{
			currGameInfo.lPlayer.xPos = rectY.x;
			currGameInfo.lPlayer.yPos = rectY.y;
			currGameInfo.lPlayer.yVel = rectVelocity;
			currGameInfo.rPlayer.xPos = mpClient->otherShapeX;
			currGameInfo.rPlayer.yPos = mpClient->otherShapeY;
			currGameInfo.rPlayer.yVel = mpClient->otherVelocity;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) mpClient->setY(-5.0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) mpClient->setY(5.0);
		if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) rectVelocity = 0.0f;
		
		/*if (rectY.y < 0)
		{
			rectVelocity = 0.0f;
			rectY.y = 0;
		}
		if (rectY.y > 600 - rect.getSize().y)
		{
			rectVelocity = 0.0f;
			rectY.y = 600 - rect.getSize().y;
		}*/
		
		rectY.y += rectVelocity;
		rect.setPosition(rect.getPosition().x, mpClient->getY());
		//mpClient->sendPaddleData(rectY.x, rectY.y, rectVelocity);

		ObjectInfo info = mpClient->getOpponentInterpolation().GetNext(mpClient->getDeltaT());
		otherRect.setPosition(sf::Vector2f(otherRect.getPosition().x, info.GetState().mY ));

		ball.setPosition(mpClient->ballX, mpClient->ballY);

		/*currPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);
		
		if (currPos.x != prevPos.x && currPos.y != prevPos.y)
		{
			otherRect.setPosition(currPos);
		}
		else
		{
			prevPos.y += mpClient->otherVelocity;
			otherRect.setPosition(prevPos);
		}*/
		
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();
		}

		// clear the window with black color
		window.clear(sf::Color::Black);

		window.draw(rect);
		window.draw(otherRect);
		window.draw(ball);

		// draw everything here...
		// window.draw(...);

		// end the current frame
		window.display();
	}

	return 0;
}