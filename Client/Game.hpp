#ifndef H_GAME
#define H_GAME

#include <iostream>
#include <cstring>
#include <mutex>
#include <regex>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Rendzu.hpp"
#include "Connect.hpp"

using namespace sf;

class Game
{
public:
    Game() 
        : _connect{1900, "192.168.0.206"}, _client(_connect)
    {
        flag_wait_game = -1;
        help();
        start();
    }

    void start()
    {
        _handle_receive = std::thread([this]{_listen();});
        while (_connect.getState() == Connect::STATE::GOOD)
        {
            std::string msg;

            std::getline(std::cin, msg);

            if (msg == "create")
            {
                std::cout << "Wait a game..." << std::endl << "Write q to exit lobby" << std::endl;
                flag_wait_game = 1;
                _connect.sendMSG(msg);
            }
            else if (msg == "q" && flag_wait_game == 1)
            {
                flag_wait_game = -1;
            }
            else if (flag_wait_game == -1)
                _connect.sendMSG(msg);

            if (msg == "exit")
                _connect.disconnectServ();
        }
        stop();
    }

    void help()
    {
        std::cout << "create - creates a new game (color is selected randomly)" << std::endl
                  << "show - demonstrates sessions that you can join (game: <id>)" << std::endl
                  << "join <id> - connect to the game with an id" << std::endl;
    }

    void stop()
    {
        _connect.disconnectServ();
        if (_handle_receive.joinable())
            _handle_receive.join();
    }
private:
    class ClientRendzy
    {
    public:
        ClientRendzy(Connect& connect)
            :  _connect(connect), flag_check_win(false)
        {}

        void move(int i, int j)
        {
            rendzy.move(i, j);
            flag_check_win = true;
        }

        void operator()(int color)
        {
            RenderWindow window(VideoMode(350, 410), "Rendzu");
            rendzy.start(color);

            Font font;
            font.loadFromFile("font.ttf");
            Text win_text, color_text;
            win_text.setFont(font);
            win_text.setCharacterSize(40);
            win_text.setStyle(Text::Bold);
            win_text.setColor(sf::Color::Red);
            win_text.setPosition(35, 100);
            std::string str = "You color: ";
            if (color == 1)
                str += "White";
            else
                str += "Black";
            color_text.setString(str);
            color_text.setFont(font);
            color_text.setCharacterSize(40);
            color_text.setStyle(Text::Bold);
            color_text.setColor(sf::Color::White);
            color_text.setPosition(30, 360);

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
                                std::string msg = "mv ";

                                auto mov = rendzy.unlockSelect();

                                msg += std::to_string(mov.first);
                                msg += " ";
                                msg += std::to_string(mov.second);
                                _connect.sendMSG(msg);
                                flag_check_win = true;
                            }
                        }
                    }
                    if (event.type == Event::Closed)
                        window.close(); 
                }

                if (flag_check_win)
                {
                    int win = rendzy.checkWin();

                    if (win != -1)
                    {
                        string str = "Win: ";
                        str += win ? "Black player" : "White player";
                        win_text.setString(str);
                    }
                    flag_check_win = false;
                }

                rendzy.checkCollision(pos);

                window.clear();
                rendzy.draw(window);

                if (rendzy.getState())
                    window.draw(win_text);
                window.draw(color_text);
                window.display();
            }
        }
    private:
        Rendzy rendzy;
        bool flag_check_win;
        Connect& _connect;
    };

    void _listen()
    {
        while (_connect.getState() == Connect::STATE::GOOD)
        {
            auto msg = _connect.getMSG();
            
            if (msg.size() > 1)
            {
                if (msg[0] == 'i')
                {
                    std::cout << msg << std::endl;
                }
                else if (msg[0] == 'r')
                {
                    if (msg[1] == 'g')
                    {

                        std::cout << "Start game" << std::endl;
                        _game_thread = std::thread([this, msg]{_client(msg[2] - '0');});
                    }
                    else 
                    {
                        std::cout << "Incorrect id" << std::endl;
                    }
                }
                else if (msg[0] == 'm')
                {
                     std::cout << "Enemy move: " << msg << std::endl;
                    int i = -1, j = -1;
                    std::string s = msg;
                    std::string delimiter = " ";

                    size_t pos = 0;
                    std::string token;
                    s.erase(0, s.find(' ') + 1);
                    while ((pos = s.find(delimiter)) != std::string::npos) {
                        token = s.substr(0, pos);

                        if (i == -1)
                        {
                            i = std::atoi(token.c_str());
                        }
                        s.erase(0, pos + delimiter.length());
                    }
                    j = std::atoi(s.c_str());
                    _client.move(i, j);
                }
            }
        }
    }

    Connect _connect;
    int flag_wait_game;
    std::thread _handle_receive, _handle_request, _game_thread;
    ClientRendzy _client;
    std::recursive_mutex _mutex_connect;
};

#endif