#ifndef PLAYER_H
#define PLAYER_H


#include <QString>


// Игрок (человек или бот)
class Player {
public:
    Player();
    Player(QString name, bool isBot = false);


    QString name;
    int score;
    bool isBot;
};


#endif // PLAYER_H
