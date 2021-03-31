# 連珠 (Rendju) Papkov Alexei 
Renju is a Japanese board game similar to Tic-tac-toe  
  
<img src=https://i.ibb.co/xDYSWB8/rendz.jpg alt="drawing" width=250>
  
The essence of the game is to collect five stones in a row (diagonal) of the same color. There are two players and they take turns on the 15x15 board

+ Make for Windows:  
>Need install SFML: https://www.sfml-dev.org/download.php  
>>Compile with g++/gcc/MSVC
+ Make for Linux:
>Need install SFML (ex. sudo Pacman -S sfml)
>>Compile server: g++ *.hpp *.cpp -o Server -lpthread  
>>Compile client: g++ *.hpp *.cpp -o Client -lpthread -lsfml-window -lsfml-system -lsfml-graphics