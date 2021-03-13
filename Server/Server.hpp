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
#include <regex>
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
                std::cout << it->first << "(" << it->second->getSocket() << ")\n";
            }

            std::cout << "delete client\n";

            auto it = _clients.find(uid);
            std::cout << it->first << "(" << it->second->getSocket() << ")\n";
            _mutex_clients.lock();
            it->second->join();
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
                std::cout << "Close client socket (" << it->second->getSocket() << ")" << std::endl;
                shutdown(it->second->getSocket(), 2);
                close(it->second->getSocket());
                it->second->join();
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

        ClientServer(int socket, std::map<int, ClientServer*>& clients)
            : _socket(socket), _clients(clients)
        {
            _handle_receive = std::thread([this]{_listen();});
        }

        void setEnemy(int enemy)
        {
            _enemy = enemy;
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
            int result = 1;
            do
            {
                char buffer[256];
                result = recv(_socket, buffer, sizeof(buffer), 0);
                std::cout << _socket << ": " << buffer << std::endl;

                if (std::string(buffer) == "show")
                {
                    std::string msg = "i";
                    auto v = Game::getInstance()->getGame();

                    for (auto& el : v)
                    {
                        msg += "Game: ";
                        msg += std::to_string(el.first);
                        msg += "\n";
                    }
                    send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
                } 
                else if (std::string(buffer) == "create")
                {
                    Game::getInstance()->createGame(_socket, -1);
                }
                else if (std::regex_match(std::string(buffer), std::regex("join [0-9]*")))
                {
                    bool flag_correct = true;
                    std::string str = std::string(buffer);
                    std::string buffer_number = "";

                    for (auto it = find(str.begin(), str.end(), ' ') + 1; it != str.end(); it++)
                    {
                        if ((*it) < '0' || (*it) > '9')
                        {
                            flag_correct = false;
                        }
                        buffer_number += (*it);
                    }

                    if (flag_correct)
                    {
                        flag_correct = Game::getInstance()->joinGame(std::atoi(buffer_number.c_str()), _socket);
                    }
                    
                    std::string msg;
                    if (!flag_correct)
                    {
                        msg = "rb";
                    }
                    else
                    {
                        auto players = Game::getInstance()->getPlayers(std::atoi(buffer_number.c_str()));
                        int color = std::rand() % 2;
                        msg = "rg";
                        msg += std::to_string(color);
                        _enemy = players.first;

                        send(players.first, msg.c_str(), sizeof(msg.c_str()), 0);
                        _clients[players.first]->setEnemy(_socket);


                        msg = "rg";
                        msg += std::to_string(1 - color);
                        
                    }
                    send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
                }
                else if (std::regex_match(std::string(buffer), std::regex("mv [0-9]* [0-9]*")))
                {
                    std::cout << "Enemy: " << _enemy << ": " << buffer << std::endl;
                    send(_enemy, buffer, sizeof(buffer), 0);
                }
                else if (std::string(buffer) == "exit")
                {
                    result = -1;
                }
            } while (result > 0);
            std::thread del_th = std::thread([this]{Server::getInstance()->deleteClient(_socket);});
            del_th.detach();
        }

        std::thread _handle_receive;
        std::map<int, ClientServer*>& _clients;
        int _socket;
        int _enemy;
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
            _clients[sock] = new ClientServer(sock, _clients);
            _mutex_clients.unlock();
        }
    }

    STATE _state;
    int _socket;
    sockaddr_in _address;
    std::thread _handle_connection;
    std::recursive_mutex _mutex_clients;
    std::map<int, ClientServer*> _clients;

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