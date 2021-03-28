#ifndef H_GAME
#define H_GAME

#include <iostream>
#include <cstring>
#include <mutex>
#include <regex>
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Rendzu.hpp"
#include "Connect.hpp"

using namespace sf;

class Game
{
public:
    Game();

    /**
     * @brief starts a thread listening to the server
     * listens for client input
     * 
     */
    void start();

    void help();

    /**
     * @brief stops the game, disconnects from the server
     * 
     */
    void stop();
private:
    /**
     * @brief listens to the server
     * 
     */
    void _listen();

    /**
     * @brief functional class, for the game
     * 
     */
    class GameClient 
    {
    public:
        GameClient(Connect& connect);

        void setActive(bool active);
        
        /**
         * @brief opponent's move
         * 
         * @param i 
         * @param j 
         */
        void move(int i, int j);

        /**
         * @brief Makes a win if the opponent is disconnected
         * 
         */
        void autoWin();

        /**
         * @brief starts the game
         * 
         * @param color - my color
         */
        void operator()(int color);
    private:
        bool _active; // window activity
        bool _check_win; // win flag
        bool _notify_server; // for correct notify
        Connect& _connection;
        Rendzy rendzy; // board
    };

    Connect _connect;
    GameClient _game;
    std::thread _handle_receive, _game_thread;
    std::recursive_mutex _mutex_connect;

    bool _flag_game;
};

#endif