#include <fstream>

#include "Game.hpp"

int main(int argc, char const *argv[])
{
    std::ifstream configuration("conf");

    std::string address;
    int port;

    configuration >> address >> port;

    Game game{address, port};
    game.start();

    return 0;
}