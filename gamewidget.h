#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <memory>
#include "gamelogic.h"
#include <QPointF>

class GameWidget : public QWidget
{
    Q_OBJECT                  // <- обязательно, чтобы moc сгенерировал код

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget() override = default;

    void setPaused(bool p) { paused = p; }

signals:
    // Сообщает MainWindow кто победил: "white", "black" или "draw"
    void gameEnded(const QString &winner);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onFrame();

private:
    GameLogic logic;
    QTimer timer;
    std::shared_ptr<Checker> selectedChecker;
    QPointF dragStart;
    bool dragging = false;
    bool paused = false;
    int left = 50, top = 50, size = 700;
    float cell() const { return size / 8.0f; }
    float radius() const { return cell() * 0.4f; }

    void playerShoot(std::shared_ptr<Checker> c, QPointF impulse);
    void checkEndCondition();
};

#endif // GAMEWIDGET_H
