#ifndef H_SERVER
#define H_SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/signal.h>
#include <map>
#include <regex>
#include <netinet/tcp.h>
#include <iostream>
#include <mutex>

#include "Game.hpp"

typedef int SOCK;

class Server
{
public:
    enum STATE
    {
        WORK,
        CLOSE,
        NOT_INIT,
        ERROR_SOCKET_INIT,
        ERROR_SOCKET_BIND
    };

    Server() 
        : _state(NOT_INIT)
    {}

    Server(Server&) = delete;

    void operator=(const Server&) = delete;

    /**
     * @brief Destroy the Server object (close server socket)
     * 
     */
    ~Server()
    {
        if (_state == WORK)
            stop();
    }

    /**
     * @brief Mandatory function for initializing server parameters
     * 
     * @param address string, ex: 192.168.1.1
     * @param port  int, ex: 8080
     */
    void init(const std::string& address, const int& port)
    {
        std::cout << "Init server on: " << address << "::" << port << std::endl;
        
        _address.sin_family = AF_INET;
        _address.sin_port = htons(port);
        inet_pton(AF_INET, address.c_str(), &(_address.sin_addr));
    }

    /**
     * @brief Outputs all users that are currently connected to the output stream
     * 
     */
    void showUser()
    {
        for (auto& client : _clients)
            std::cout << "Socket: " << client.first << std::endl;
    }

    /**
     * @brief Displays games that are currently active
     * 
     */
    void showGame()
    {
        auto games = Game::getInstance()->getAllGames();

        for (auto& game : games)
            std::cout << "UID: " <<  game.first << ": (" << game.second.first << ", " << game.second.second << ")" << std::endl;
    }

    /**
     * @brief The function that activates the server, the bind occurs on the parameters set earlier
     * 
     * @return int - error code (if != 0 -> error)
     */
    int start()
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);

        int err_bind;

        std::cout << "Socket init: ";
        if (_socket < 0)
        {
            std::cout << "Bad (" << _socket << ")" << std::endl;
            _state = ERROR_SOCKET_INIT;
            return _state;
        }
        std::cout << "Good" << std::endl;

        err_bind = bind(_socket, (sockaddr*)&_address, sizeof(_address));

        std::cout << "Socket bind: ";
        if (err_bind < 0)
        {
            std::cout << "Bad (" << err_bind << ")" << std::endl;
            _state = ERROR_SOCKET_BIND;
            return _state;
        }
        std::cout << "Good" << std::endl;

        listen(_socket, 1);
        _state = WORK;

        _handle_accept = std::thread([this]{_listenAccept();});

        return _state;
    }

    /**
     * @brief Disconnects all clients and closes the server socket
     * 
     */
    void stop()
    {
        _state = CLOSE;

        shutdown(_socket, 2);
        close(_socket);

        if (_handle_accept.joinable())
            _handle_accept.join();

        for (auto it = _clients.begin(); it != _clients.end(); it = _clients.begin())
        {
            delete (*it).second;
            _clients.erase(it);
        }
    }

    static Server* getInstance();

    /**
     * @brief Disconnects the user by socket
     * 
     * @param socket 
     */
    void deleteClient(SOCK socket)
    {
        _mutex_clients.lock();
        std::cout << "delete client " << socket << std::endl;
        _clients[socket]->~Client();
        _clients.erase(socket);

        _mutex_clients.unlock();
    }

    /**
     * @brief Causes an auto win message to be sent when the opponent is disabled
     * 
     * @param socket winner
     */
    void autoWin(SOCK socket)
    {
        _mutex_clients.lock();

        _clients[socket]->autoWin();

        _mutex_clients.unlock();
    }

    /**
     * @brief Get the State object
     * 
     * @return STATE 
     */
    STATE getState() const 
    {
        return _state;
    }
private:
    /**
     * @brief activates keepAlive on the client socket
     * 
     * @param sock 
     */
    void enableKeepalive(SOCK socket)
    {
        int yes = 1;
        setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
        int idle = 1;
        setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

        int interval = 1;
        setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

        int maxpkt = 10;
        setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
    }

    /**
     * @brief Listens to the server socket and processes new connections. Adds them to map _clients
     * 
     */
    void _listenAccept()
    {
        while (_state == WORK)
        {
            sockaddr_in cs_addr;
            socklen_t cs_addrsize = sizeof (cs_addr);

            SOCK sock = accept(_socket, (sockaddr*)&cs_addr, &cs_addrsize);
            if (sock <= 0)
                continue;

            enableKeepalive(sock);
            std::cout << "Accept: " << inet_ntoa(cs_addr.sin_addr) << "(" << sock << ")" << std::endl;

            _mutex_clients.lock();
            _clients[sock] = new Client(sock);
            _mutex_clients.unlock();
        }
    }

    class Client
    {
    public:
        /**
         * @brief Construct a new Client object. constructs a class and starts a message processing thread
         * 
         * @param socket client
         */
        Client(SOCK socket)
            : _socket(socket), _game_uid(-1)
        {
            _handle_message = std::thread([this]{_listen();});
        }

        /**
         * @brief Destroy the Client object. Closes the client socket and waits for the message processing flow to stop
         * 
         */
        ~Client()
        {
            std::cout << "Close: " << _socket << std::endl;
            shutdown(_socket, 2);
            close(_socket);

            join();
        }

        /**
         * @brief bridge for _handle_message thread
         * 
         */
        void join()
        {
            if (_handle_message.joinable())
                _handle_message.join();
        }

        /**
         * @brief sends a message to the client about the auto win
         * 
         */
        void autoWin()
        {
            _game_uid = -1;
            
            char buf[] = {"aw"};
            send(_socket, buf, sizeof(buf), 0);
        }
    private:
        /**
         * @brief Sends all active games IDs to the client
         * 
         */
        void _show()
        {
            auto session = Game::getInstance()->getAllGames();

            std::string msg = "";

            for (auto& game : session)
            {
                msg += std::to_string(game.first);
                msg += "\n";
            }

            send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
        }

        /**
         * @brief Creates a new active game where the client will wait for the opponent
         * 
         */
        void _create()
        {
            _game_uid = Game::getInstance()->createGame(_socket);
        }

        /**
         * @brief connects the client to the active game and informs the opponent about it.
         * And determines the colors of the players
         * 
         * @param game_uid active game
         */
        void _join(int game_uid)
        {
            Game::getInstance()->joinGame(game_uid, _socket);
            _game_uid = game_uid;

            std::string msg = "s";
            int color = rand() % 2;

            msg += std::to_string(color);
            send(Game::getInstance()->getGame(game_uid).first, msg.c_str(), sizeof(msg.c_str()), 0);

            msg = "s";
            msg += std::to_string(1 - color);
            send(_socket, msg.c_str(), sizeof(msg.c_str()), 0);
        }

        /**
         * @brief Sends the opponent information about the client's new move
         * 
         * @param mv - for ex mv 2 4
         */
        void _move(const std::string& mv)
        {
            auto game = Game::getInstance()->getGame(_game_uid);

            int enemy = -1;

            if (game.first != _socket && game.first != -1)
                enemy = game.first;
            if (game.second != _socket && game.second != -1)
                enemy = game.second;
            send(enemy, mv.c_str(), sizeof(mv.c_str()), 0);
        }


        /**
         * @brief Deletes the active game due to disconnection, informs the opponent about it (if he was) call autoWin
         * 
         */
        void _clearGame()
        {
            std::cout << "clear game " << _game_uid << std::endl;
            if (_game_uid != -1)
            {
                auto game = Game::getInstance()->getGame(_game_uid);

                int enemy = -1;

                if (game.first != _socket && game.first != -1)
                    enemy = game.first;
                if (game.second != _socket && game.second != -1)
                    enemy = game.second;

                if (enemy != -1)
                    Server::getInstance()->autoWin(enemy);
                Game::getInstance()->deleteGame(_game_uid);
                _game_uid = -1;       
            }
        }


        /**
         * @brief Processes messages from the client (in _handlge_message thread)
         * 
         */
        void _listen()
        {
            int result = 1;

            do
            {
                char buffer[256];

                result = recv(_socket, buffer, sizeof(buffer), 0);
                std::string msg = std::string(buffer);

                if (msg == "")
                    continue;

                if (msg == "show")
                    _show();
                else if (msg == "create" && _game_uid == -1)
                    _create();
                else if (msg == "exit_lob")
                    _clearGame();
                else if (std::regex_match(msg, std::regex("join [0-9]*")))
                    _join(std::atoi(msg.substr(4, msg.length() - 4).c_str()));
                else if (std::regex_match(msg, std::regex("mv [0-9]* [0-9]*")))
                    _move(msg);

                std::cout << _socket << ": " << buffer << std::endl;
            } while (result > 0);
            _clearGame();

            std::thread del_th = std::thread([this]{Server::getInstance()->deleteClient(_socket);});
            del_th.detach();
        }

        std::thread _handle_message;
        SOCK _socket;

        int _game_uid;
    };

    static Server* _server;

    sockaddr_in _address;
    SOCK _socket;

    STATE _state;
    std::map<SOCK, Client*> _clients;
    std::recursive_mutex _mutex_clients;
    std::thread _handle_accept;
};

Server* Server::_server = nullptr;

Server* Server::getInstance()
{
    if (_server == nullptr)
        _server = new Server();
    return _server;
}

#endif