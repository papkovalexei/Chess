#ifndef H_CONNECT
#define H_CONNECT

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>

class Connect
{
public:
    enum STATE
    {
        GOOD,
        BAD,
        CLOSE
    };

    Connect(int port, const std::string& address)
    {
        connectServ(port, address);
    }

    STATE getState() const
    {
        return _state;
    }

    void sendMSG(std::string& msg)
    {
        send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
    }

    void connectServ(int port, const std::string& address)
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);

        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(1900);
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

    void disconnectServ()
    {
        _state = CLOSE;
        shutdown(_socket, 2);
        close(_socket);
    }

    ~Connect()
    {
        shutdown(_socket, 2);
        close(_socket);
    }
private:
    STATE _state;
    int _socket;
    sockaddr_in _addr;
};

#endif