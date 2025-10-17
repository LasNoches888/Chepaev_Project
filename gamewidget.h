#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include <QColor>
#include <QPointF>

struct Checker {
    QPointF pos;
    QPointF vel;
    QColor color;
    bool alive = true;
};

float length(QPointF v);

class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private slots:
    void updatePhysics();

private:
    std::vector<Checker> checkers;
    Checker* selected = nullptr;
    QPointF dragStart;
    bool dragging = false;
    float radius = 25;
    QTimer timer;
    void initCheckers();
};

#endif // GAMEWIDGET_H
