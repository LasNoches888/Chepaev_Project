#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QPainter>
#include <QPointF>
#include <QVector>
#include <QColor>
#include <memory>

class Checker {
public:
    QPointF pos;
    QPointF vel;
    QColor color;
    bool alive;

    Checker(QPointF p = QPointF(0, 0), QColor c = Qt::white)
        : pos(p), vel(0, 0), color(c), alive(true) {}
};

class GameLogic
{
public:
    GameLogic();

    float boardLeft;
    float boardTop;
    float boardSize;

    void initBoard();
    void update(float dt);
    void drawBoard(QPainter *p);
    void shoot(int checkerIndex, const QPointF &force);
    void updateCheckerPositions(); // НОВЫЙ МЕТОД

    bool checkGameOver() const;
    QString winner() const;
    bool isMoving() const;
    int getCheckerAtPosition(const QPointF &pos) const;
    QColor getCheckerColor(int index) const;
    QPointF getCheckerPosition(int index) const;
    QVector<int> getBlackCheckers() const;
    QVector<int> getWhiteCheckers() const;
    float evaluateMove(int checkerIndex, const QPointF &force) const;

    const QVector<std::shared_ptr<Checker>>& getCheckers() const { return checkers; }
    int getCheckerCount() const { return checkers.size(); }
    bool isCheckerAlive(int index) const {
        return index >= 0 && index < checkers.size() && checkers[index]->alive;
    }

private:
    QVector<std::shared_ptr<Checker>> checkers;
    QString winnerColor;
    bool gameOver;

    // Сохраняем исходные позиции шашек относительно доски
    QVector<QPointF> initialPositions;

    float length(const QPointF &v) const;
    void handleCollisions();
    QPointF predictPosition(const QPointF &startPos, const QPointF &startVel, float time) const;
};

#endif // GAMELOGIC_H
