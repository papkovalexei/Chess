#ifndef H_CCLIENT
#define H_CCLIENT
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

class ClientServer
{
public:
    ClientServer() {}

    ClientServer(int socket)
        : _socket(socket)
    {
        _handle_receive = std::thread([this]{_listen();});
    }


    void join()
    {
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
        } while (result > 0);
    }

    std::thread _handle_receive;
    int _socket;
};

#endif