#include <SFML/Graphics.hpp>
#include <iostream>

#include "Rendzu.hpp"

using namespace std;
using namespace sf;

int main()
{	
	RenderWindow window(VideoMode(350, 344), "Rendzu");
    Rendzy rendzy;

    Font font;
    font.loadFromFile("font.ttf");
    Text win_text;
    win_text.setFont(font);
    win_text.setCharacterSize(40);
    win_text.setStyle(Text::Bold);
    win_text.setColor(Color::Red);
    win_text.setPosition(35, 100);

	while (window.isOpen())
	{
        Vector2i pixelPos = Mouse::getPosition(window);
		Vector2f pos = window.mapPixelToCoords(pixelPos);

		Event event;
		while (window.pollEvent(event))
		{
            if (!rendzy.getState())
            {
                if (event.type == Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        rendzy.lockSelect();
                    }
                }
                else if (event.type == Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        rendzy.unlockSelect();
                        int win = rendzy.checkWin();

                        if (win != -1)
                        {
                            string str = "Win: ";
                            str += win ? "Black player" : "White player";
                            win_text.setString(str);
                        }
                    }
                }
            }
			if (event.type == Event::Closed)
				window.close(); 
		}

        rendzy.checkCollision(pos);

        window.clear();
        rendzy.draw(window);

        if (rendzy.getState())
            window.draw(win_text);

		window.display();
	}
 
	return 0;
}