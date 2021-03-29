#pragma warning(disable : 4996)

#include "Game.hpp"

Game::Game(const std::string& address, const int& port) 
    : _connect{port, address}, _flag_game(false), _game(_connect)
{
    help();
    start();
}

void Game::start()
{
    _handle_receive = std::thread([this]{_listen();});
    while (_connect.getState() == Connect::STATE::GOOD)
    {
        std::string msg;

        std::getline(std::cin, msg);

        if (!_flag_game)
            _connect.sendMSG(msg);
        else if (msg == "exit")
            _connect.sendMSG(msg);
        else if (_flag_game && msg == "q")
        {
            _game.setActive(false);
            msg = "exit_lobby";
            _flag_game = false;
            _connect.sendMSG(msg);
        }

        if (msg == "create" || std::regex_match(msg, std::regex("join [0-9]*")))            
            _flag_game = true;

        if (msg == "exit")
            _connect.disconnectServ();
    }
    stop();
}


void Game::help()
{
    std::cout << "create - creates a new game (color is selected randomly)" << std::endl
                << "show - demonstrates sessions that you can join (game: <id>)" << std::endl
                << "join <id> - connect to the game with an id" << std::endl;
}

void Game::stop()
{
    _game.setActive(false);
    _connect.disconnectServ();

    if (_handle_receive.joinable())
        _handle_receive.join();
    if (_game_thread.joinable())
        _game_thread.join();
}

void Game::_listen()
{
    while (_connect.getState() == Connect::STATE::GOOD)
    {
        auto msg = _connect.getMSG();
        std::cout << msg << std::endl;
        
        if (msg.size() >= 2)
        {
            if (msg[0] == 's')
            {
                if (_game_thread.joinable())
                    _game_thread.join();

                _game_thread = std::thread([this, msg]{_game(msg[1] - '0');});
            }
            else if (msg == "aw")
            {
                _game.autoWin();
                std::cout << "You win!!!" << std::endl;
                _flag_game = false;
            }
            else if (std::regex_match(msg, std::regex("mv [0-9]* [0-9]*")))
            {
                int i, j;

                msg.erase(0, msg.find(' ') + 1);

                i = std::atoi(msg.substr(0, msg.find(' ')).c_str());
                j = std::atoi(msg.erase(0, msg.find(' ') + 1).c_str());

                _game.move(i, j);
            }
        }
    }
}

Game::GameClient::GameClient(Connect& connect)
    : _connection(connect)
{}

void Game::GameClient::setActive(bool active)
{
    _active = active;
}

void Game::GameClient::move(int i, int j)
{
    rendzy.move(i, j);
    _check_win = true;
}

void Game::GameClient::autoWin()
{
    _active = false;
}

void Game::GameClient::operator()(int color)
{
    _active = true;
    _check_win = false;
    _notify_server = false;

    RenderWindow window(VideoMode(350, 400), "Rendzu");
    rendzy.start(color);

    Font font;
    font.loadFromFile("font.ttf");
    Text win_text, player_text;
    win_text.setFont(font);
    win_text.setCharacterSize(40);
    win_text.setStyle(Text::Bold);
    win_text.setColor(Color::Red);
    win_text.setPosition(35, 100);

    player_text.setFont(font);
    player_text.setCharacterSize(30);
    player_text.setStyle(Text::Bold);
    player_text.setColor(Color::White);
    player_text.setPosition(35, 350);
    
    std::string color_str = "Your color: ";
    color_str += color ? "white" : "black";
    player_text.setString(color_str);
    while (window.isOpen())
    {
        Vector2i pixelPos = Mouse::getPosition(window);
        Vector2f pos = window.mapPixelToCoords(pixelPos);

        Event event;
        while (window.pollEvent(event))
        {
            if (!rendzy.getState())
            {
                if (event.type == Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        rendzy.lockSelect();
                    }
                }
                else if (event.type == Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        auto mv = rendzy.unlockSelect();
                        _check_win = true;

                        std::string msg_move = "mv ";
                        msg_move += std::to_string(mv.first);
                        msg_move += " ";
                        msg_move += std::to_string(mv.second);

                        /**
                         * @brief send my move to opponents
                         * 
                         */
                        _connection.sendMSG(msg_move);
                    }
                }
            }
            if (event.type == Event::Closed)
                window.close(); 
        }

        if (_check_win)
        {
            int win = rendzy.checkWin();

            if (win != -1)
            {
                string str = "Win: ";
                str += win ? "Black player" : "White player";
                win_text.setString(str);
            }
            _check_win = false;
        }

        rendzy.checkCollision(pos);

        window.clear();
        rendzy.draw(window);
        window.draw(player_text);
        if (rendzy.getState())
        {
            window.draw(win_text);

            if (!_notify_server)
            {
                std::string notify = "end";
                _connection.sendMSG(notify);
                _notify_server = true;
            }
        }

        window.display();

        if (!_active)
            window.close();
    }
}