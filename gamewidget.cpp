#include "gamewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent), logic(), dragging(false)
{
    setFixedSize(left * 2 + size, top * 2 + size);
    logic.initBoard();
    connect(&timer, &QTimer::timeout, this, &GameWidget::onFrame);
    timer.start(16);
}

void GameWidget::onFrame()
{
    if (paused) return;

    const float dt = 1.0f;
    logic.update(dt);

    // Бот ход каждый ~90 кадров
    static int counter = 0;
    if (++counter > 90) {
        logic.botMakeMove();
        counter = 0;
    }

    checkEndCondition();
    update();
}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // фон
    p.fillRect(rect(), QColor(60, 60, 60));

    // доска
    p.setBrush(QColor(220, 180, 130));
    p.setPen(Qt::NoPen);
    p.drawRect(left, top, size, size);

    // сетка
    p.setPen(QPen(QColor(200, 160, 110), 1));
    for (int i = 0; i <= 8; ++i) {
        p.drawLine(left + i * cell(), top, left + i * cell(), top + size);
        p.drawLine(left, top + i * cell(), left + size, top + i * cell());
    }

    // шашки
    for (auto &c : logic.checkers) {
        if (!c->alive) continue;
        p.setBrush(c->color);
        p.setPen(Qt::NoPen);
        p.drawEllipse(c->pos, radius(), radius());
    }

    // линия силы при перетаскивании
    if (dragging && selectedChecker) {
        p.setPen(QPen(Qt::red, 2));
        QPointF mouse = mapFromGlobal(QCursor::pos());
        p.drawLine(selectedChecker->pos, mouse);

        QPointF impulse = dragStart - mouse;
        float power = std::sqrt(impulse.x()*impulse.x() + impulse.y()*impulse.y());
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 10));
        p.drawText(10, height() - 10, QString("Сила: %1").arg((int)power));
    }

    // HUD — счёт и подсказка по управлению
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 12, QFont::Bold));
    QString score = QString("Белые: %1    Чёрные(бот): %2")
                        .arg(logic.whitePlayer.score)
                        .arg(logic.blackPlayer.score);
    p.drawText(10, 20, score);

    p.setFont(QFont("Arial", 10));
    p.drawText(10, 40, "ЛКМ: выбрать + тянуть, Меню: Новая игра/Пауза/Настройки");
}

void GameWidget::mousePressEvent(QMouseEvent *event)
{
    if (paused) return;
    QPointF mouse = event->pos();

    for (auto &c : logic.checkers) {
        if (!c->alive) continue;
        if (c->color != Qt::white) continue; // игрок — белые
        float dx = mouse.x() - c->pos.x();
        float dy = mouse.y() - c->pos.y();
        float dist2 = dx*dx + dy*dy;
        if (dist2 <= radius() * radius()) {
            selectedChecker = c;
            dragStart = mouse;
            dragging = true;
            selectedChecker->vel = QPointF(0,0);
            return;
        }
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (paused) return;
    if (!dragging || !selectedChecker) {
        dragging = false;
        selectedChecker = nullptr;
        return;
    }
    QPointF mouse = event->pos();
    QPointF impulse = (dragStart - mouse);
    playerShoot(selectedChecker, impulse);
    dragging = false;
    selectedChecker = nullptr;
}

void GameWidget::playerShoot(std::shared_ptr<Checker> c, QPointF impulse)
{
    if (!c || !c->alive) return;
    const float maxForce = 1200.0f;
    float L = std::sqrt(impulse.x()*impulse.x() + impulse.y()*impulse.y());
    if (L > 0.0f) {
        float factor = std::min(maxForce, L);
        float scale = 0.03f;
        QPointF vel = (impulse / L) * (factor * scale);
        c->vel = vel;
    }
}

void GameWidget::checkEndCondition()
{
    bool anyWhite = false;
    bool anyBlack = false;
    for (auto &c : logic.checkers) {
        if (!c->alive) continue;
        if (c->color == Qt::white) anyWhite = true;
        if (c->color == Qt::black) anyBlack = true;
    }

    if (!anyWhite && !anyBlack) {
        emit gameEnded(QString("draw"));
    } else if (!anyWhite) {
        emit gameEnded(QString("black"));
    } else if (!anyBlack) {
        emit gameEnded(QString("white"));
    }
}

