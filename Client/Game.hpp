#ifndef H_GAME
#define H_GAME

#include <iostream>
#include <cstring>

#include "Connect.hpp"

class Game
{
public:
    Game() 
        : _connect{1900, "192.168.0.206"}
    {
        start();
    }

    void start()
    {
        while (_connect.getState() == Connect::STATE::GOOD)
        {
            std::string msg;

            std::getline(std::cin, msg);

            if (msg == "exit")
            {
                _connect.disconnectServ();
            }
            else
                _connect.sendMSG(msg);
        }
    }
private:
    Connect _connect;
};

#endif