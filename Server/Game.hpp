#ifndef H_GAME
#define H_GAME

#include <iostream>
#include <vector>

class Game
{
public:
    Game() {}

    Game(Game&) = delete;
    void operator=(const Game&) = delete;

    static Game* getInstance();

    void createGame(int first, int second)
    {
        _session.push_back(std::pair<int, int>(first, second));
    }

    std::vector<std::pair<int, int>> getGame()
    {
        return _session;
    }
private:
    std::vector<std::pair<int, int>> _session;
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