#ifndef H_GAME
#define H_GAME

#include <iostream>
#include <vector>
#include <map>

class Game
{
public:
    Game() {}

    Game(Game&) = delete;
    void operator=(const Game&) = delete;

    static Game* getInstance();

    void createGame(int first, int second)
    {
        _session[_uid++] = std::pair<int, int>(first, second);
    }

    std::pair<int, int> getPlayers(int key)
    {
        return _session[key];
    }

    bool joinGame(int key, int second)
    {
        if (_session.count(key) == 0)
            return false;
        _session[key].second = second;
        return true;
    }

    std::map<int, std::pair<int, int>> getGame()
    {
        return _session;
    }
private:
    int _uid = 0;
    std::map<int, std::pair<int, int>> _session;
    static Game* _game;
};

Game* Game::_game = nullptr;

Game *Game::getInstance()
{
    if (_game == nullptr)
        _game = new Game();

    return _game;
}

#endif