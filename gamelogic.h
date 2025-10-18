#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>
#include <memory>
#include "checker.h"
#include "player.h"
#include <QPointF>

class GameLogic {
public:
    GameLogic();

    void initBoard();
    void update(float dt);

    std::vector<std::shared_ptr<Checker>> checkers;
    Player whitePlayer;
    Player blackPlayer;

    int boardLeft;
    int boardTop;
    int boardSize;

    void botMakeMove();

private:
    void handleCollisions();
    float length(const QPointF &v) const;
};

#endif // GAMELOGIC_H
