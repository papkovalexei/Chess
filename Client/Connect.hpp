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
    int _socket;
    sockaddr_in _addr;
};

#endif