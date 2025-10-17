#include "gamewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

float length(QPointF v) {
    return std::sqrt(v.x() * v.x() + v.y() * v.y());
}

GameWidget::GameWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(800, 800);
    initCheckers();
    connect(&timer, &QTimer::timeout, this, &GameWidget::updatePhysics);
    timer.start(16);
}

void GameWidget::initCheckers() {
    for (int i = 0; i < 8; i++) {
        for (int row = 0; row < 2; row++) {
            Checker c1{{100.0 + i * 80.0, 100.0 + row * 80.0}, {0.0, 0.0}, Qt::white};
            checkers.push_back(c1);
            Checker c2{{100.0 + i * 80.0, 600.0 + row * 80.0}, {0.0, 0.0}, Qt::black};
            checkers.push_back(c2);
        }
    }
}

void GameWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Доска
    p.fillRect(rect(), QColor(60, 60, 60));
    p.setBrush(QColor(220, 180, 130));
    p.drawRect(50, 50, 700, 700);

    // Шашки
    for (auto &c : checkers) {
        if (!c.alive) continue;
        p.setBrush(c.color);
        p.drawEllipse(c.pos, radius, radius);
    }

    // Линия силы
    if (dragging && selected) {
        p.setPen(Qt::red);
        p.drawLine(selected->pos, mapFromGlobal(QCursor::pos()));
    }
}

void GameWidget::mousePressEvent(QMouseEvent *e) {
    QPointF mouse = e->pos();
    for (auto &c : checkers) {
        if (!c.alive) continue;
        if (length(mouse - c.pos) < radius) {
            selected = &c;
            dragStart = mouse;
            dragging = true;
            break;
        }
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *e) {
    if (dragging && selected) {
        QPointF mouse = e->pos();
        QPointF impulse = dragStart - mouse;
        selected->vel = impulse * 0.1;
    }
    dragging = false;
    selected = nullptr;
}

void GameWidget::updatePhysics() {
    for (auto &c : checkers) {
        if (!c.alive) continue;
        c.pos += c.vel;
        c.vel *= 0.97;
        if (length(c.vel) < 0.1) c.vel = {0, 0};

        if (c.pos.x() < 50 || c.pos.x() > 750 || c.pos.y() < 50 || c.pos.y() > 750)
            c.alive = false;
    }

    for (size_t i = 0; i < checkers.size(); i++) {
        for (size_t j = i + 1; j < checkers.size(); j++) {
            auto &a = checkers[i];
            auto &b = checkers[j];
            if (!a.alive || !b.alive) continue;
            QPointF diff = b.pos - a.pos;
            float dist = length(diff);
            if (dist < 2 * radius && dist > 0) {
                QPointF n = diff / dist;
                float overlap = 2 * radius - dist;
                a.pos -= n * overlap / 2.0;
                b.pos += n * overlap / 2.0;
                std::swap(a.vel, b.vel);
            }
        }
    }

    update();
}
