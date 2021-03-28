#include "Game.hpp"

Game::Game() 
    : _uid(0)
{}

Game* Game::_game = nullptr;

Game* Game::getInstance()
{
    if (_game == nullptr)
        _game = new Game();
    return _game;
}

int Game::createGame(int socket_player)
{
    _uid++;

    _sessions[_uid].first = socket_player;
    _sessions[_uid].second = -1;

    return _uid;
}

void Game::joinGame(int key, int player)
{
    _sessions[key].second = player;
}

const std::pair<int, int> Game::getGame(int key)
{
    return _sessions[key];
}

void Game::deleteGame(int key)
{
    if (_sessions.count(key) > 0)
        _sessions.erase(key);
}

const std::map<int, std::pair<int, int>> Game::getAllGames() const
{
    return _sessions;
}