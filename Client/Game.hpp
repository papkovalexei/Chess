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
        : _connect{1900, "192.168.0.206"}, _flag_game(false)
    {
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

            if (!_flag_game)
                _connect.sendMSG(msg);
            else if (msg == "exit")
                _connect.sendMSG(msg);
            else if (_flag_game && msg == "q")
            {
                msg = "exit_lobby";
                _flag_game = false;
                _connect.sendMSG(msg);
            }

            if (msg == "create" || std::regex_match(msg, std::regex("join [0-9]*")))            
                _flag_game = true;

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
    void _listen()
    {
        while (_connect.getState() == Connect::STATE::GOOD)
        {
            auto msg = _connect.getMSG();
            std::cout << msg << std::endl;
            
            if (msg.size() >= 2)
            {
                if (msg[0] == 's')
                {
                }
            }
        }
    }

    Connect _connect;
    std::thread _handle_receive;
    std::recursive_mutex _mutex_connect;

    bool _flag_game;
};

#endif