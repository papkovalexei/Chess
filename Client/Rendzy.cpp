#include "Rendzu.hpp"

Rendzy::Rendzy() 
{}

void Rendzy::move(int i, int j)
{
    _circle_rect_position[i][j].setFillColor(_player ? Color::Black : Color::White);
    _board_position[i][j] = _player;
    _player = !_player;
}

void Rendzy::start(bool choose)
{
    _image_board.loadFromFile("board.png");
    _texture_board.loadFromImage(_image_board);
    _sprite_board.setTexture(_texture_board);
    _sprite_board.setPosition(0, 0);

    const float dx = 21.5, dy = 21.5, start = 17;

    float x_start = start, y_start = 14;
    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            _circle_rect_position[i][j].setPosition(x_start, y_start);
            _circle_rect_position[i][j].setRadius(9);
            _circle_rect_position[i][j].setFillColor(Color::Transparent);

            x_start += dx;

            _board_position[i][j] = -1;
        }
        x_start = start;
        y_start += dy;
    }

    _select_rect = pair<int, int>(0, 0);
    _lock_select = -1;
    _player = 0;
    _me = !choose;
    _win = false;
}

bool Rendzy::getState() const
{
    return _win;
}

void Rendzy::checkCollision(Vector2f mouse)
{
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
        {
            if (_circle_rect_position[i][j].getGlobalBounds().contains(mouse) && _board_position[i][j] == -1 && _lock_select == -1 && _me == _player)
            {
                if (_select_rect.first != -1)
                    _circle_rect_position[_select_rect.first][_select_rect.second].setFillColor(Color::Transparent);
                _circle_rect_position[i][j].setFillColor(_player ? Color::Black : Color::White);
                _select_rect = pair<int, int>(i, j);
            }
        }
}

bool Rendzy::checkWin()
{
    bool mbWinner = !_player;
    cout << mbWinner << endl;
    int stack = 0, max_stack = -1;
    // step 1 - horz
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
        {
            if (_board_position[i][j] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
        }
    if (max_stack >= 5)
    {
        _win = true;
        return mbWinner;
    }

    stack = 0;
    max_stack = -1;
    // step 2 - vert
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
        {
            if (_board_position[j][i] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
        }
    if (max_stack >= 5)
    {
        _win = true;
        return mbWinner;
    }

    stack = 0;
    max_stack = -1;
    // step 3 - diag-

    for (int i = 4; i < 15; i++)
    {
        int temp_i = i;

        for (int j = 0; j <= i; j++)
        {
            if (_board_position[temp_i][j] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
            temp_i--;
        }
    }
    stack = 0;

    for (int i = 0; i <= 10; i++)
    {
        int temp_i = i;

        for (int j = 14; j >= i; j--)
        {
            if (_board_position[temp_i][j] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
            temp_i++;
        }
    }

    if (max_stack >= 5)
    {
        _win = true;
        return mbWinner;
    }     

    stack = 0;
    max_stack = -1;
    // step 4 - diag+
    for (int j = 4; j <= 14; j++)
    {
        int temp_j = j;
        for (int i = 14; i >= 14 - j; i--)
        {
            if (_board_position[i][temp_j] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
            temp_j--;
        }
    }
    
    stack = 0;

    for (int i = 14; i >= 4; i--)
    {
        int temp_i = i;

        for (int j = 14; j >= 14 - i; j--)
        {
            if (_board_position[temp_i][j] == mbWinner)
            {
                stack++;
            }
            else
            {
                max_stack = max(max_stack, stack);
                stack = 0;
            }
            temp_i--;
        }
    }

    if (max_stack >= 5)
    {
        _win = true;
        return mbWinner;
    }
    return -1;
}

void Rendzy::lockSelect()
{
    _lock_select = 1;
}

std::pair<int, int> Rendzy::unlockSelect()
{
    auto temp = _select_rect;
    _lock_select = -1;
    _playerMove();
    return temp;
}

void Rendzy::draw(RenderWindow& window)
{
    window.draw(_sprite_board);

    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            window.draw(_circle_rect_position[i][j]);
}

void Rendzy::_playerMove()
{
    if (_select_rect.first != -1 && _board_position[_select_rect.first][_select_rect.second] == -1)
    {
        _board_position[_select_rect.first][_select_rect.second] = _player;
        _circle_rect_position[_select_rect.first][_select_rect.second].setFillColor(_player ? Color::Black : Color::White);
        _select_rect.first = -1;
        _player = !_player;
    }
}