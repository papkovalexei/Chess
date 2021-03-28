#ifndef H_RENDZU
#define H_RENDZU

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Connect.hpp"

using namespace sf;
using namespace std;

class Rendzy
{
public:
    Rendzy() 
   {}

    /**
     * @brief Records the opponent's move in the game matrix.
     * Gives access to the player's next move (changing the _player flag)
     * 
     * @param i 
     * @param j 
     */
    void move(int i, int j)
    {
        _circle_rect_position[i][j].setFillColor(_player ? Color::Black : Color::White);
        _board_position[i][j] = _player;
        _player = !_player;
    }

    /**
     * @brief Initializes all texture positions, as well as variables responsible for the game
     * 
     * @param choose  player color
     */
    void start(bool choose)
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

    /**
     * @brief Get the _win object
     * 
     * @return true - game over
     * @return false - the game continues
     */
    bool getState()
    {
        return _win;
    }

    /**
     * @brief Checks for a collision between the cursor and the stone. If there is a collision, then it draws a texture
     * 
     * @param mouse 
     */
    void checkCollision(Vector2f mouse)
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

    /**
     * @brief checks the position for a win
     * 
     * @return bool - who won
     */
    bool checkWin()
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

    /**
     * @brief Pressing the LMB
     * 
     */
    void lockSelect()
    {
        _lock_select = 1;
    }

    /**
     * @brief Unpressed the LMB
     * 
     * @return std::pair<int, int> - position (i, j)
     */
    std::pair<int, int> unlockSelect()
    {
        auto temp = _select_rect;
        _lock_select = -1;
        _playerMove();
        return temp;
    }

    /**
     * @brief Drawing the game
     * 
     * @param window 
     */
    void draw(RenderWindow& window)
    {
        window.draw(_sprite_board);

        for (int i = 0; i < 15; i++)
            for (int j = 0; j < 15; j++)
                window.draw(_circle_rect_position[i][j]);
    }
private:
    /**
     * @brief drawing a stone after a move, changing the move flag, entering a position in the matrix
     * 
     */
    void _playerMove()
    {
        if (_select_rect.first != -1 && _board_position[_select_rect.first][_select_rect.second] == -1)
        {
            _board_position[_select_rect.first][_select_rect.second] = _player;
            _circle_rect_position[_select_rect.first][_select_rect.second].setFillColor(_player ? Color::Black : Color::White);
            _select_rect.first = -1;
            _player = !_player;
        }
    }

    Image _image_board;
    Texture _texture_board;
    Sprite _sprite_board;

    int _board_position[15][15]; // game position
    bool _player; // now player
    bool _me; // my color
    bool _win; // the game continues?
    pair<int, int> _select_rect; // now the LMB position
    int _lock_select; // for mouse
    CircleShape _circle_rect_position[15][15]; // stone position and color
};

#endif