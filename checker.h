#ifndef CHECKER_H
#define CHECKER_H


#include <QPointF>
#include <QColor>


// Структура, описывающая шашку
class Checker {
public:
    Checker();
    Checker(QPointF position, QColor color);


    QPointF pos; // позиция (центр)
    QPointF vel; // скорость
    QColor color; // цвет
    bool alive; // в игре ли
    int id; // уникальный id (для отладки)
};


#endif // CHECKER_H
