#include "checker.h"


static int g_id_counter = 0;


Checker::Checker()
    : pos(0.0, 0.0), vel(0.0, 0.0), color(Qt::white), alive(true), id(++g_id_counter)
{
}


Checker::Checker(QPointF position, QColor color)
    : pos(position), vel(0.0, 0.0), color(color), alive(true), id(++g_id_counter)
{
}
