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
#include <map>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sqlite3.h>
#include <unistd.h>
#include <iostream>
#include <mutex>

#include "Game.hpp"

class Server
{
public:
    Server() {}

    void init(int port, const std::string& address) 
    {
        _uid = 0;
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

    void deleteClient(int uid)
    {
        if (_state == WORK)
        {
            for (auto it = _clients.begin(); it != _clients.end(); it++)
            {
                std::cout << it->first << "(" << it->second.getSocket() << ")\n";
            }

            std::cout << "delete client\n";

            auto it = _clients.find(uid);
            std::cout << it->first << "(" << it->second.getSocket() << ")\n";
            _mutex_clients.lock();
            it->second.join();
            _clients.erase(it);
            _mutex_clients.unlock();
        }
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
                std::cout << "Close client socket (" << it->second.getSocket() << ")" << std::endl;
                shutdown(it->second.getSocket(), 2);
                close(it->second.getSocket());
                it->second.join();
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

    static Server* getInstance();
private:
    class ClientServer
    {
    public:
        ClientServer() {}

        ClientServer(int socket, int uid)
            : _socket(socket), _uid(uid)
        {
            _handle_receive = std::thread([this]{_listen();});
        }

        

        void join()
        {
            if (_handle_receive.joinable())
                _handle_receive.join();
        }

        int getSocket()
        {
            return _socket;
        }

    private:
        void _listen()
        {
            char buffer[256];
            int result = 1;
            do
            {
                result = recv(_socket, buffer, sizeof(buffer), 0);
                std::cout << socket << ": " << buffer << std::endl;

                if (std::string(buffer) == "show")
                {
                    auto v = Game::getInstance()->getGame();
                    std::cout << "show!!!" << v.size() << std::endl;

                    for (auto& el : v)
                    {
                        std::cout << "(" << el.first << ", " << el.second << ")" << std::endl;
                    }
                } 
                else if (std::string(buffer) == "create")
                {
                    Game::getInstance()->createGame(_socket, -1);
                }
            } while (result > 0);
            std::thread del_th = std::thread([this]{Server::getInstance()->deleteClient(_uid);});
            del_th.detach();
        }

        std::thread _handle_receive;
        int _socket;
        int _uid;
    };

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
            _clients[_uid] = ClientServer(sock, _uid);
            _uid++;
            _mutex_clients.unlock();
        }
    }

    STATE _state;
    int _socket;
    int _uid;
    sockaddr_in _address;
    std::thread _handle_connection;
    std::recursive_mutex _mutex_clients;
    std::map<int, ClientServer> _clients;

    static Server* _server;
};

Server* Server::_server = nullptr;

Server* Server::getInstance()
{
    if (_server == nullptr)
        _server = new Server();
    return _server;
}

#endif