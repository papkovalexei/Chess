#ifndef H_CONNECT
#define H_CONNECT

#ifdef _WIN32
#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

#include <Winsock2.h>
#include <WS2tcpip.h>

#else

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#endif

#include <iostream>
#include <vector>
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

    Connect(int port, const std::string& address);

    ~Connect();

    STATE getState() const;

    void sendMSG(std::string& msg);

    /**
     * @brief Reads data from the socket input stream, and returns a string
     * 
     * @return std::string 
     */
    std::string getMSG();

    /**
     * @brief Connects a socket to the server. If it fails, the _state variable will have the corresponding flag
     * 
     * @param port ex. 2020
     * @param address ex. 192.168.1.1
     */
    void connectServ(int port, const std::string& address);

    void disconnectServ();

private:
    STATE _state;
    sockaddr_in _addr;
#ifdef _WIN32
    SOCKET _socket;
    WSAData _w_data;
#else
    int _socket;
    sockaddr_in _addr;
#endif
};

#endif