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
    send(_socket, msg.c_str(), msg.size(), 0);
}

std::string Connect::getMSG()
{
    std::vector<char> buffer(256);

    int result = 1;
    
    result = recv(_socket, buffer.data(), buffer.size(), 0);
    
    if (result <= 0)
        _state = BAD;
    return buffer.data();
}

void Connect::connectServ(int port, const std::string& address)
{
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &_w_data);
#endif
    _socket = socket(AF_INET, SOCK_STREAM, 0);

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
#ifdef _WIN32
    _addr.sin_addr.s_addr = inet_addr(address.c_str());
#else
    inet_pton(AF_INET, address.c_str(), &(_addr.sin_addr));
#endif

#ifdef _WIN32
    if (connect(_socket, (SOCKADDR*)&_addr, sizeof(_addr)) == SOCKET_ERROR)
#else
    if (connect(_socket, (sockaddr *)&_addr, sizeof(_addr)) < 0)
#endif
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
#ifdef _WIN32
    closesocket(_socket);
#else
    close(_socket);
#endif
}

Connect::~Connect()
{
    shutdown(_socket, 2);
#ifdef _WIN32
    closesocket(_socket);
#else
    close(_socket);
#endif
}