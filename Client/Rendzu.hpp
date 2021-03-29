#ifndef H_RENDZU
#define H_RENDZU

#ifdef _WIN32
#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")
#endif

#include <SFML/Graphics.hpp>
#include <iostream>

#include "Connect.hpp"

using namespace sf;
using namespace std;

class Rendzy
{
public:
    Rendzy();

    /**
     * @brief Records the opponent's move in the game matrix.
     * Gives access to the player's next move (changing the _player flag)
     * 
     * @param i 
     * @param j 
     */
    void move(int i, int j);

    /**
     * @brief Initializes all texture positions, as well as variables responsible for the game
     * 
     * @param choose  player color
     */
    void start(bool choose);

    /**
     * @brief Get the _win object
     * 
     * @return true - game over
     * @return false - the game continues
     */
    bool getState() const;

    /**
     * @brief Checks for a collision between the cursor and the stone. If there is a collision, then it draws a texture
     * 
     * @param mouse 
     */
    void checkCollision(Vector2f mouse);

    /**
     * @brief checks the position for a win
     * 
     * @return bool - who won
     */
    bool checkWin();

    /**
     * @brief Pressing the LMB
     * 
     */
    void lockSelect();

    /**
     * @brief Unpressed the LMB
     * 
     * @return std::pair<int, int> - position (i, j)
     */
    std::pair<int, int> unlockSelect();

    /**
     * @brief Drawing the game
     * 
     * @param window 
     */
    void draw(RenderWindow& window);
private:
    /**
     * @brief drawing a stone after a move, changing the move flag, entering a position in the matrix
     * 
     */
    void _playerMove();
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