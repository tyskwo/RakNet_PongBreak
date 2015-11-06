#include <SFML/Graphics.hpp>
#include "Server.h"

double calcDifferenceInMS(LARGE_INTEGER from, LARGE_INTEGER to) const
{
		double difference = (double)(to.QuadPart - from.QuadPart) / (double)(mFrequency.QuadPart);
		return difference * 1000;
}

int main()
{
	// create the window
	sf::RenderWindow window(sf::VideoMode(800, 600), "PONGBREAK server");
	window.setFramerateLimit(60);

	//timer variables
	LARGE_INTEGER mStartTime;
	LARGE_INTEGER mEndTime;
	LARGE_INTEGER mFrequency;

	QueryPerformanceFrequency(&mFrequency);
	QueryPerformanceCounter(&mStartTime);

	Server* mpServer = new Server("200");

	// run the program as long as the window is open
	while (window.isOpen())
	{
		QueryPerformanceCounter(&mEndTime);
		double timeSinceLastUpdate = calcDifferenceInMS(mStartTime, mEndTime);
		mpServer->update(timeSinceLastUpdate);

		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
		}

		// clear the window with black color
		window.clear(sf::Color::Black);

		// draw everything here...
		// window.draw(...);

		// end the current frame
		window.display();
	}

	return 0;
}