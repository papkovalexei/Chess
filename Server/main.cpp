#include "Server.hpp"

void func()
{
    Server::getInstance()->init(1900, "192.168.0.206");
    Server::getInstance()->start();
    std::this_thread::sleep_for(std::chrono::seconds(500));
    Server::getInstance()->stop();
}

int main(int argc, char const *argv[])
{
    std::thread ser(func);

    int stop;
    std::cin >> stop;

    ser.join();

    return 0;
}
