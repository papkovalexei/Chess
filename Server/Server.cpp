
#ifdef _WIN32
#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")
#include <string.h>
#endif

#include "Server.hpp"

Server::Server() 
    : _state(NOT_INIT)
{}

Server::~Server()
{
    if (_state == WORK)
        stop();
}

Server* Server::_server = nullptr;

Server* Server::getInstance()
{
    if (_server == nullptr)
        _server = new Server();
    return _server;
}

void Server::init(const std::string& address, const int& port)
{
    std::cout << "Init server on: " << address << "::" << port << std::endl;
    _address.sin_family = AF_INET;
    _address.sin_port = htons(port);
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &_w_data);
    _address.sin_addr.S_un.S_addr = inet_addr(address.c_str());
#else
    inet_pton(AF_INET, address.c_str(), &(_address.sin_addr));
#endif
}

int Server::start()
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);

    int err_bind;

    std::cout << "Socket init: ";
#ifdef _WIN32
    if (static_cast<int>(_socket) == SOCKET_ERROR)
#else
    if (_socket < 0)
#endif
    {
        std::cout << "Bad (" << _socket << ")" << std::endl;
        _state = ERROR_SOCKET_INIT;
        return _state;
    }
    std::cout << "Good" << std::endl;

    err_bind = bind(_socket, (sockaddr*)&_address, sizeof(_address));

    std::cout << "Socket bind: ";
#ifdef _WIN32
    if (err_bind == SOCKET_ERROR)
#else
    if (err_bind < 0)
#endif
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

void Server::stop()
{
    _state = CLOSE;

    shutdown(_socket, 2);
#ifdef _WIN32
    closesocket(_socket);
    WSACleanup();
#else
    close(_socket);
#endif

    if (_handle_accept.joinable())
        _handle_accept.join();

    for (auto it = _clients.begin(); it != _clients.end(); it = _clients.begin())
    {
        delete (*it).second;
        _clients.erase(it);
    }
}

#ifdef _WIN32
void Server::deleteClient(SOCKET socket)
#else
void Server::deleteClient(SOCK socket)
#endif
{
    _mutex_clients.lock();
    std::cout << "delete client " << socket << std::endl;
    _clients[socket]->~Client();
    _clients.erase(socket);

    _mutex_clients.unlock();
}

#ifdef _WIN32
void Server::autoWin(SOCKET socket)
#else
void Server::autoWin(SOCK socket)
#endif
{
    _mutex_clients.lock();

    _clients[socket]->autoWin();

    _mutex_clients.unlock();
}

#ifdef _WIN32
Server::Client::Client(SOCKET socket)
#else
Server::Client::Client(SOCK socket)
#endif
    : _socket(socket), _game_uid(-1)
{
    _handle_message = std::thread([this]{_listen();});
}

void Server::_listenAccept()
{
    while (_state == WORK)
    {
#ifdef _WIN32
        SOCKADDR_IN cs_addr;
#else
        sockaddr_in cs_addr;
#endif
        socklen_t cs_addrsize = sizeof (cs_addr);
#ifdef _WIN32
        SOCKET sock = accept(_socket, (sockaddr*)&cs_addr, &cs_addrsize);

        if (sock == SOCKET_ERROR)
            continue;
#else
        SOCK sock = accept(_socket, (sockaddr*)&cs_addr, &cs_addrsize);

        if (sock <= 0)
            continue;
#endif

        enableKeepalive(sock);
        std::cout << "Accept: " << inet_ntoa(cs_addr.sin_addr) << "(" << sock << ")" << std::endl;

        _mutex_clients.lock();
        _clients[sock] = new Client(sock);
        _mutex_clients.unlock();
    }
}

Server::Client::~Client()
{
    std::cout << "Close: " << _socket << std::endl;
    shutdown(_socket, 2);
#ifdef _WIN32
    closesocket(_socket);
#else
    close(_socket);
#endif

    join();
}

#ifdef _WIN32

void Server::enableKeepalive(SOCKET socket)
{
    BOOL yes = 1;
    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&yes, sizeof(int));
    BOOL idle = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&idle, sizeof(int));

    int interval = 1;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&interval, sizeof(int));

    int maxpkt = 10;
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, (char *)&maxpkt, sizeof(int));
}

#else

void Server::enableKeepalive(SOCK socket)
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

#endif

void Server::showUser()
{
    for (auto& client : _clients)
        std::cout << "Socket: " << client.first << std::endl;
}

void Server::showGame()
{
    auto games = Game::getInstance()->getAllGames();

    for (auto& game : games)
        std::cout << "UID: " <<  game.first << ": (" << game.second.first << ", " << game.second.second << ")" << std::endl;
}


Server::STATE Server::getState() const 
{
    return _state;
}

void Server::Client::join()
{
    if (_handle_message.joinable())
        _handle_message.join();
}
         
void Server::Client::autoWin()
{
    _game_uid = -1;
    
    char buf[] = {"aw"};
    send(_socket, buf, sizeof(buf), 0);
}

void Server::Client::_show()
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

void Server::Client::_create()
{
    _game_uid = Game::getInstance()->createGame(_socket);
}

void Server::Client::_join(int game_uid)
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

void Server::Client::_move(const std::string& mv)
{
    auto game = Game::getInstance()->getGame(_game_uid);

    int enemy = -1;

    if (game.first != _socket && game.first != -1)
        enemy = game.first;
    if (game.second != _socket && game.second != -1)
        enemy = game.second;
    send(enemy, mv.c_str(), mv.size(), 0);
}

void Server::Client::_clearGame()
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

void Server::Client::_endGame()
{
    _mutex_game.lock();
    std::cout << "Delete: " << _game_uid << std::endl;
    if (_game_uid != -1)
        Game::getInstance()->deleteGame(_game_uid);
    _mutex_game.unlock();
    _game_uid = -1;
}

void Server::Client::_listen()
{
    int result = 1;

    do
    {
        std::vector<char> buffer(256);

        result = recv(_socket, buffer.data(), buffer.size(), 0);
        std::string msg = buffer.data();

        if (msg == "")
            continue;

        if (msg == "show")
            _show();
        else if (msg == "create" && _game_uid == -1)
            _create();
        else if (msg == "exit_lobby")
            _clearGame();
        else if (std::regex_match(msg, std::regex("join [0-9]*")))
            _join(std::atoi(msg.substr(4, msg.length() - 4).c_str()));
        else if (std::regex_match(msg, std::regex("mv [0-9]* [0-9]*")))
            _move(msg);
        else if (msg == "end")
            _endGame();

        std::cout << _socket << ": " << buffer.data() << std::endl;
    } while (result > 0);
    _clearGame();

    std::thread del_th = std::thread([this]{Server::getInstance()->deleteClient(_socket);});
    del_th.detach();
}