#include "Server.hpp"

void func()
{
    Server server(1900, "192.168.0.206");
    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    server.stop();
}

int main(int argc, char const *argv[])
{
    std::thread ser(func);

    int stop;
    std::cin >> stop;

    ser.join();

    return 0;
}
