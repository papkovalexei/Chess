#include "Connect.hpp"

Connect::Connect(int port, const std::string& address)
{
    connectServ(port, address);
}

Connect::STATE Connect::getState() const
{
    return _state;
}

void Connect::sendMSG(std::string& msg)
{
    send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
}

std::string Connect::getMSG()
{
    char buffer[1024];
    int result = 1;
    
    result = recv(_socket, buffer, sizeof(buffer), 0);
    
    if (result <= 0)
        _state = BAD;
    return buffer;
}

void Connect::connectServ(int port, const std::string& address)
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &(_addr.sin_addr));

    if (connect(_socket, (sockaddr *)&_addr, sizeof(_addr)) < 0)
    {
        std::cout << "Connection: Bad" << std::endl;
        _state = BAD;
    }
    else
    {
        std::cout << "Connection: Good" << std::endl;
        _state = GOOD;
    }
}

void Connect::disconnectServ()
{
    _state = CLOSE;
    shutdown(_socket, 2);
    close(_socket);
}

Connect::~Connect()
{
    shutdown(_socket, 2);
    close(_socket);
}