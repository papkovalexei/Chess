#ifndef H_GAME
#define H_GAME

#include <map>
#include <iostream>

class Game
{
public:
    Game();

    Game(Game&) = delete;
    void operator=(const Game&) = delete;

    static Game* getInstance();

    /**
     * @brief Create new active game session
     * 
     * @param socket_player first player 
     * @return int IDs game
     */
    int createGame(int socket_player);

    /**
     * @brief Adds an oponent to the active game
     * 
     * @param key IDs game
     * @param player Second player
     */
    void joinGame(int key, int player);

    /**
     * @brief Get active game
     * 
     * @param key IDs game
     * @return const std::pair<int, int> 
     */
    const std::pair<int, int> getGame(int key);

    /**
     * @brief Delete active game
     * 
     * @param key IDs
     */
    void deleteGame(int key);

    /**
     * @brief Get the All active game
     * 
     * @return const std::map<int, std::pair<int, int>> 
     */
    const std::map<int, std::pair<int, int>> getAllGames() const;

private:
    static Game* _game;
    
    std::map<int, std::pair<int, int>> _sessions;
    int _uid;
};

#endif 