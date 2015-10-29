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
	Client* mpClient = new Client(char_type, "localhost", "200");


	sf::CircleShape circle(50);
	// set the shape color to green
	circle.setFillColor(sf::Color(100, 250, 50));

	sf::CircleShape circleOther(50);
	// set the shape color to green
	circleOther.setFillColor(sf::Color(200, 100, 100));

	// run the program as long as the window is open
	while (window.isOpen())
	{
		mpClient->update();

		sf::Vector2i mousePos = sf::Mouse::getPosition(window);

		if (mousePos.x > 800 - circle.getRadius() * 2)	mousePos.x = 800 - circle.getRadius() * 2;
		if (mousePos.y > 600 - circle.getRadius() * 2)	mousePos.y = 600 - circle.getRadius() * 2;
		if (mousePos.x <   0)							mousePos.x =   0;
		if (mousePos.y <   0)							mousePos.y =   0;

		circle.setPosition(sf::Vector2f(mousePos));
		mpClient->sendShapePacket(mousePos.x, mousePos.y);
		sf::Vector2f otherPosition = sf::Vector2f(mpClient->otherShapeX, mpClient->otherShapeY);
		circleOther.setPosition(otherPosition);




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

		window.draw(circle);
		window.draw(circleOther);

		// draw everything here...
		// window.draw(...);

		// end the current frame
		window.display();
	}

	return 0;
}