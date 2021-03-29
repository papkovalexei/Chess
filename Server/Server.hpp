#ifndef H_SERVER
#define H_SERVER

#ifdef _WIN32

#include <WinSock2.h>
#include <winsock.h>
#include <WS2tcpip.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/signal.h>
#include <unistd.h>

#endif

#include <vector>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <regex>
#include <iostream>
#include <mutex>
#include <vector>
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

    Server();

    Server(Server&) = delete;

    void operator=(const Server&) = delete;

    /**
     * @brief Destroy the Server object (close server socket)
     * 
     */
    ~Server();

    /**
     * @brief Mandatory function for initializing server parameters
     * 
     * @param address string, ex: 192.168.1.1
     * @param port  int, ex: 8080
     */
    void init(const std::string& address, const int& port);

    /**
     * @brief Outputs all users that are currently connected to the output stream
     * 
     */
    void showUser();

    /**
     * @brief Displays games that are currently active
     * 
     */
    void showGame();

    /**
     * @brief The function that activates the server, the bind occurs on the parameters set earlier
     * 
     * @return int - error code (if != 0 -> error)
     */
    int start();

    /**
     * @brief Disconnects all clients and closes the server socket
     * 
     */
    void stop();

    static Server* getInstance();

    /**
     * @brief Disconnects the user by socket
     * 
     * @param socket 
     */
#ifdef _WIN32
    void deleteClient(SOCKET socket);
#else
    void deleteClient(SOCK socket);
#endif

    /**
     * @brief Causes an auto win message to be sent when the opponent is disabled
     * 
     * @param socket winner
     */
#ifdef _WIN32
    void autoWin(SOCKET socket);
#else
    void autoWin(SOCK socket);
#endif

    /**
     * @brief Get the State object
     * 
     * @return STATE 
     */
    STATE getState() const;
private:
    /**
     * @brief activates keepAlive on the client socket
     * 
     * @param sock 
     */
#ifdef _WIN32
    void enableKeepalive(SOCKET socket);
#else
    void enableKeepalive(SOCK socket);
#endif
    /**
     * @brief Listens to the server socket and processes new connections. Adds them to map _clients
     * 
     */
    void _listenAccept();

    class Client
    {
    public:
        /**
         * @brief Construct a new Client object. constructs a class and starts a message processing thread
         * 
         * @param socket client
         */
#ifdef _WIN32
        Client(SOCKET socket);
#else
        Client(SOCK socket);
#endif

        /**
         * @brief Destroy the Client object. Closes the client socket and waits for the message processing flow to stop
         * 
         */
        ~Client();

        /**
         * @brief bridge for _handle_message thread
         * 
         */
        void join();

        /**
         * @brief sends a message to the client about the auto win
         * 
         */
        void autoWin();
    private:
        /**
         * @brief Sends all active games IDs to the client
         * 
         */
        void _show();

        /**
         * @brief Creates a new active game where the client will wait for the opponent
         * 
         */
        void _create();

        /**
         * @brief connects the client to the active game and informs the opponent about it.
         * And determines the colors of the players
         * 
         * @param game_uid active game
         */
        void _join(int game_uid);

        /**
         * @brief Sends the opponent information about the client's new move
         * 
         * @param mv - for ex mv 2 4
         */
        void _move(const std::string& mv);

        /**
         * @brief Deletes the active game due to disconnection, informs the opponent about it (if he was) call autoWin
         * 
         */
        void _clearGame();

        /**
         * @brief 
         * 
         */
        void _endGame();

        /**
         * @brief Processes messages from the client (in _handlge_message thread)
         * 
         */
        void _listen();

#ifdef _WIN32
        SOCKET _socket;
#else
        SOCK _socket;
#endif
        std::thread _handle_message;
        std::recursive_mutex _mutex_game;
        int _game_uid;
    };

    static Server* _server;
#ifdef _WIN32
    SOCKET _socket;
    WSAData _w_data;
    SOCKADDR_IN _address;
    std::map<SOCKET, Client*> _clients;
#else
    sockaddr_in _address;
    SOCK _socket;
    std::map<SOCK, Client*> _clients;
#endif
    STATE _state;
    std::recursive_mutex _mutex_clients;
    std::thread _handle_accept;
};

#endif