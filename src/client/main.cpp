#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include "Client.h"

int main()
{
	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

	srand(time(0));
	int randPortNumber = rand() % (250 - 201 + 1) + 201;
	std::stringstream converter;
	converter << randPortNumber;
	std::string temp_str = converter.str();
	char* char_type = (char*)temp_str.c_str();

	std::string ipAddress = "";
	std::cout << "Input server IP or \"localhost\": ";
	std::cin >> ipAddress;
	Client* mpClient = new Client(char_type, ipAddress.c_str(), "200");


	while (!mpClient->getConnected()) { mpClient->update(); }


	sf::RectangleShape rect(sf::Vector2f(20, 100));
	sf::RectangleShape otherRect(sf::Vector2f(20, 100));

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

	sf::Vector2f rectY = rect.getPosition();

	float rectVelocity = 0.0f;

	mpClient->update();

	sf::Vector2f prevPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);
	sf::Vector2f currPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);

	// run the program as long as the window is open
	while (window.isOpen())
	{
		mpClient->update();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && rectY.y >= 20) rectVelocity = -5.0f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && rectY.y <= 600 - rect.getSize().y - 20) rectVelocity = 5.0f;
		if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) rectVelocity = 0.0f;

		rectY.y += rectVelocity;
		rect.setPosition(rectY);
		mpClient->sendShapePacket(rectY.x, rectY.y, rectVelocity);

		currPos = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);
		
		if (currPos.x != prevPos.x && currPos.y != prevPos.y)
		{
			otherRect.setPosition(currPos);
		}
		else
		{
			prevPos.y += mpClient->otherVelocity;
			otherRect.setPosition(prevPos);
		}
		
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

		// draw everything here...
		// window.draw(...);

		// end the current frame
		window.display();
	}

	return 0;
}