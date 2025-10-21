#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include "gamelogic.h"

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    enum Difficulty { Easy = 0, Medium = 1, Hard = 2 };
    void setBotDifficulty(Difficulty d) { difficulty = d; }
    Difficulty botDifficulty() const { return difficulty; }

signals:
    void gameEnded(const QString &winner);
    void backToMenuClicked();

private slots:
    void onFrame();
    void makeBotMove();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    GameLogic logic;
    QTimer gameTimer;
    bool dragging;
    bool playerTurn;
    int selectedChecker;
    QPointF dragStart;
    QPointF currentMouse;
    bool menuButtonHovered;

    Difficulty difficulty = Medium; // по умолчанию

    void updateBoardGeometry();
};

#endif // GAMEWIDGET_H
