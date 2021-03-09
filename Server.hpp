#ifndef H_SERVER
#define H_SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <thread>
#include <arpa/inet.h>
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sqlite3.h>
#include <unistd.h>
#include <iostream>
#include <mutex>

#include "Client.hpp"

class Server
{
public:
    Server(int port, const std::string& address) 
    {
        std::cout << "Init socket param on: " << address << "::" << port << std::endl;
        _address.sin_family = AF_INET;
        _address.sin_port = htons(port);
        inet_pton(AF_INET, address.c_str(), &(_address.sin_addr));

        _state = CLOSE;
    }

    ~Server()
    {
        if (_state == WORK)
            stop();
    }

    void start()
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);

        int err_bind;

        std::cout << "Socket init: ";
        if (_socket < 0)
        {
            std::cout << "Bad (" << _socket << ")" << std::endl;
            _state = ERROR_SOCKET_INIT;
            return;
        }
        std::cout << "Good" << std::endl;

        err_bind = bind(_socket, (sockaddr*)&_address, sizeof(_address));

        std::cout << "Socket bind: ";
        if (err_bind < 0)
        {
            std::cout << "Bad (" << err_bind << ")" << std::endl;
            _state = ERROR_SOCKET_BIND;
            return;
        }
        std::cout << "Good" << std::endl;

        listen(_socket, 1);
        _state = WORK;

        _handle_connection = std::thread([this]{_handle_accept();});
    }

    void stop()
    {
        if (_state == WORK)
        {
            std::cout << "Close server socket" << std::endl;
            shutdown(_socket, 2);
            close(_socket);
            _state = CLOSE;
            _handle_connection.join();

            for (auto it = _clients.begin(); it != _clients.end(); it = _clients.begin())
            {
                std::cout << "Close client socket (" << it->getSocket() << ")" << std::endl;
                shutdown(it->getSocket(), 2);
                close(it->getSocket());
                it->join();
                _clients.erase(it);
            }
            std::cout << "Server down" << std::endl;
        }
    }

    enum STATE
    {
        WORK,
        CLOSE,
        ERROR_SOCKET_BIND,
        ERROR_SOCKET_INIT,
        ERROR_SOCKET_KEEPALIVE
    };
private:
    void enable_keepalive(int sock)
    {
        int yes = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
        int idle = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

        int interval = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

        int maxpkt = 10;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
    }

    void _handle_accept()
    {
        while (_state == WORK)
        {
            sockaddr_in cs_addr;
            socklen_t cs_addrsize = sizeof (cs_addr);

            int sock = accept(_socket, (sockaddr*)&cs_addr, &cs_addrsize);
            if (sock <= 0)
                continue;

            enable_keepalive(sock);
            std::cout << "Accept: " << inet_ntoa(cs_addr.sin_addr) << "(" << sock << ")" << std::endl;

            _mutex_clients.lock();
            _clients.push_back(ClientServer(sock));
            _mutex_clients.unlock();
        }
    }

    STATE _state;
    int _socket;
    sockaddr_in _address;
    std::thread _handle_connection;
    std::recursive_mutex _mutex_clients;

    std::vector<ClientServer> _clients;
};

#endif