#include <fstream>

#include "Server.hpp"

int main(int argc, char const *argv[])
{
    std::ifstream configuration("conf");

    std::string address;
    int port;

    configuration >> address >> port;

    Server::getInstance()->init(address, port);
    Server::getInstance()->start();

    std::string command = "";

    while (command != "exit")
    {
        std::getline(std::cin, command);

        if (command == "show_user")
        {
            Server::getInstance()->showUser();
        }
        else if (command == "show_game")
        {
            Server::getInstance()->showGame();
        }
    }

    Server::getInstance()->stop();

    return 0;
}
