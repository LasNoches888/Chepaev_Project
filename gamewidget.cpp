#include "gamewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>
#include <cmath>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent),
    logic(),
    gameTimer(this),
    dragging(false),
    playerTurn(true),
    selectedChecker(-1),
    menuButtonHovered(false),
    difficulty(Medium)
{
    setMinimumSize(600, 600);
    setMouseTracking(true);

    // Попытка загрузить фон (тот же ресурс, что используется в меню)
    bgPixmap = QPixmap(":/images/menu_bg.jpg");

    // Инициализируем геометрию/доску
    updateBoardGeometry();
    // initBoard вызываем только при старте (если доска пуста)
    logic.initBoard();

    connect(&gameTimer, &QTimer::timeout, this, &GameWidget::onFrame);
    gameTimer.start(16); // ~60 FPS

    // Синхронизация сложности в логике
    logic.setBotDifficulty(static_cast<BotDifficulty>(difficulty));
}

QSize GameWidget::sizeHint() const
{
    return QSize(800, 800);
}

void GameWidget::setBotDifficulty(Difficulty d)
{
    difficulty = d;
    logic.setBotDifficulty(static_cast<BotDifficulty>(d));
}

void GameWidget::updateBoardGeometry()
{
    int w = width();
    int h = height();
    int minSide = qMin(w, h);
    int newBoardSize = qMax(400, static_cast<int>(minSide * 0.8));

    // Центрирование
    int newBoardLeft = (w - newBoardSize) / 2;
    int newBoardTop = (h - newBoardSize) / 2;

    // Если размер изменился значительно и шашки не движутся:
    // - если доска пуста (или только что создана) -> initBoard()
    // - если шашки уже есть -> не инициализируем заново, а пересчитываем позиции относительно новой доски
    if (std::abs(logic.boardSize - newBoardSize) > 50.0f && !logic.isMoving()) {
        logic.boardLeft = newBoardLeft;
        logic.boardTop = newBoardTop;
        logic.boardSize = newBoardSize;

        // Если ещё нет шашек (начало игры) — инициализируем
        if (logic.getCheckerCount() == 0) {
            logic.initBoard();
        } else {
            // просто перерасставляем текущие шашки по их относительным координатам
            logic.updateCheckerPositions();
        }
    } else {
        logic.boardLeft = newBoardLeft;
        logic.boardTop = newBoardTop;
        logic.boardSize = newBoardSize;
    }

    qDebug() << "Геометрия обновлена. Окно:" << w << "x" << h;
    qDebug() << "Доска:" << logic.boardLeft << logic.boardTop << logic.boardSize;
}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Рисуем фон (если есть ресурс) — сначала фон, затем доска
    if (!bgPixmap.isNull()) {
        p.drawPixmap(rect(), bgPixmap);
    } else {
        p.fillRect(rect(), QColor(44, 62, 80));
    }

    // Обновляем геометрию перед отрисовкой (на случай изменения размера)
    updateBoardGeometry();

    // Рисуем доску и шашки через GameLogic
    logic.drawBoard(&p);

    // Отрисовка UI: счёт, кнопка меню, индикатор хода и линия прицеливания
    int whiteCount = logic.getWhiteCheckers().size();
    int blackCount = logic.getBlackCheckers().size();

    // Панель счёта
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 160));
    p.drawRoundedRect(10, 10, 220, 72, 8, 8);

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.setBrush(Qt::white);
    p.drawEllipse(20, 28, 18, 18);
    p.drawText(50, 43, QString::fromUtf8("Белые: %1").arg(whiteCount));

    p.setBrush(Qt::black);
    p.drawEllipse(20, 50, 18, 18);
    p.setPen(Qt::white);
    p.drawText(50, 65, QString::fromUtf8("Черные: %1").arg(blackCount));

    // Кнопка "В меню"
    QRect menuButtonRect(width() - 140, 12, 128, 40);
    p.setPen(Qt::NoPen);
    p.setBrush(menuButtonHovered ? QColor(255,255,255,250) : QColor(255,255,255,220));
    p.drawRoundedRect(menuButtonRect, 8, 8);
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.drawText(menuButtonRect, Qt::AlignCenter, QString::fromUtf8("В меню"));

    // Линия прицеливания (если игрок тянет)
    if (dragging && selectedChecker >= 0 && logic.isCheckerAlive(selectedChecker)) {
        QPointF checkerPos = logic.getCheckerPosition(selectedChecker);
        p.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(checkerPos, currentMouse);

        QPointF direction = currentMouse - checkerPos;
        float length = std::hypot(direction.x(), direction.y());
        if (length > 0) {
            QPointF unitDir = direction / length;
            QPointF perpendicular(-unitDir.y(), unitDir.x());

            QPointF arrow1 = currentMouse - unitDir * 20 + perpendicular * 8;
            QPointF arrow2 = currentMouse - unitDir * 20 - perpendicular * 8;

            p.drawLine(currentMouse, arrow1);
            p.drawLine(currentMouse, arrow2);
        }
    }

    // Индикатор хода
    QString turnText = playerTurn ? QString::fromUtf8("🎯 Ваш ход (белые)") : QString::fromUtf8("🤖 Ход противника (черные)");
    QRect turnRect(width() / 2 - 160, height() - 70, 320, 44);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0,0,0,160));
    p.drawRoundedRect(turnRect, 10, 10);
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 14, QFont::Bold));
    p.drawText(turnRect, Qt::AlignCenter, turnText);
}

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;

    QRect menuButtonRect(width() - 140, 12, 128, 40);
    if (menuButtonRect.contains(e->pos())) {
        emit backToMenuClicked();
        return;
    }

    if (!playerTurn || logic.isMoving()) return;

    selectedChecker = -1;
    const float cell = logic.boardSize / 8.0f;
    const float radius = cell * 0.4f;

    const auto& checkers = logic.getCheckers();
    for (int i = 0; i < checkers.size(); ++i) {
        const auto& c = checkers[i];
        if (!c->alive || c->color != Qt::white) continue;

        float dist = std::hypot(e->pos().x() - c->pos.x(), e->pos().y() - c->pos.y());
        if (dist <= radius) {
            selectedChecker = i;
            break;
        }
    }

    if (selectedChecker >= 0) {
        dragging = true;
        dragStart = logic.getCheckerPosition(selectedChecker);
        currentMouse = e->pos();
        update();
    }
}

void GameWidget::mouseMoveEvent(QMouseEvent *e)
{
    QRect menuButtonRect(width() - 140, 12, 128, 40);
    bool wasHovered = menuButtonHovered;
    menuButtonHovered = menuButtonRect.contains(e->pos());
    if (wasHovered != menuButtonHovered) update();

    if (dragging && selectedChecker >= 0) {
        currentMouse = e->pos();
        update();
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return;
    if (!(dragging && selectedChecker >= 0)) return;

    dragging = false;

    QPointF checkerPos = logic.getCheckerPosition(selectedChecker);
    QPointF direction = checkerPos - e->pos();

    // Умеренная сила игрока + пределы
    const float PLAYER_FORCE_MULT = 3.0f;
    const float MAX_FORCE = 450.0f;
    QPointF rawForce = direction * PLAYER_FORCE_MULT;
    float len = std::hypot(rawForce.x(), rawForce.y());
    if (len > MAX_FORCE) rawForce *= (MAX_FORCE / len);

    const float MIN_FORCE = 10.0f;
    if (len >= MIN_FORCE) {
        logic.shoot(selectedChecker, rawForce);
        playerTurn = false; // передаём ход боту
    }

    selectedChecker = -1;
    update();
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateBoardGeometry();
    update();
}

void GameWidget::onFrame()
{
    // Физический шаг
    logic.update(0.016f);

    // Если шашки всё ещё двигаются — ждём
    if (logic.isMoving()) {
        update();
        return;
    }

    // Проверка конца игры
    if (logic.checkGameOver()) {
        QString w = logic.winner();
        if (w != "none") emit gameEnded(w);
        return;
    }

    // Ход бота если очередь за ним
    if (!playerTurn) {
        makeBotMove();
    }

    update();
}

// makeBotMove оставляем как в вашей текущей реализации (вызов логики бота затем shoot + playerTurn = true)
void GameWidget::makeBotMove()
{
    if (playerTurn || logic.isMoving() || logic.checkGameOver()) return;

    QVector<int> blackCheckers = logic.getBlackCheckers();
    if (blackCheckers.isEmpty()) {
        playerTurn = true;
        return;
    }

    // Попытка получить ход от движка
    BotMove bm = logic.findBestMove(Qt::black);
    if (bm.checkerIndex >= 0) {
        logic.shoot(bm.checkerIndex, bm.force);
        playerTurn = true;
        return;
    }

    // fallback (если findBestMove не дал результата)
    QVector<int> whiteCheckers = logic.getWhiteCheckers();

    float angleNoiseDeg = 18.0f;
    float forceNoisePct = 0.25f;
    int candidatesPerChecker = 3;
    switch (difficulty) {
    case Easy:   angleNoiseDeg = 40.0f; forceNoisePct = 0.5f;  candidatesPerChecker = 1; break;
    case Medium: angleNoiseDeg = 18.0f; forceNoisePct = 0.25f; candidatesPerChecker = 3; break;
    case Hard:   angleNoiseDeg = 6.0f;  forceNoisePct = 0.10f; candidatesPerChecker = 6; break;
    default: break;
    }

    struct Candidate { int checkerIndex; QPointF force; float score; };
    QVector<Candidate> candidates;
    candidates.reserve(blackCheckers.size() * candidatesPerChecker);

    for (int bi : blackCheckers) {
        if (!logic.isCheckerAlive(bi)) continue;

        QPointF startPos = logic.getCheckerPosition(bi);

        QPointF bestTarget = startPos;
        if (!whiteCheckers.isEmpty()) {
            float bestD = 1e9f;
            for (int wi : whiteCheckers) {
                QPointF wp = logic.getCheckerPosition(wi);
                float d = std::hypot(wp.x() - startPos.x(), wp.y() - startPos.y());
                if (d < bestD) { bestD = d; bestTarget = wp; }
            }
        } else {
            bestTarget = QPointF(startPos.x(), startPos.y() + 1.0f);
        }

        QPointF dir = bestTarget - startPos;
        float len = std::hypot(dir.x(), dir.y());
        if (len > 0.0f) dir /= len;
        else dir = QPointF(0.0f, 1.0f);

        for (int c = 0; c < candidatesPerChecker; ++c) {
            float frac = (candidatesPerChecker > 1) ? (float)c / (candidatesPerChecker - 1) : 0.5f;
            float angleOffset = (frac - 0.5f) * 2.0f * angleNoiseDeg;
            float angleRad = angleOffset * (3.14159265f / 180.0f);
            float cosA = std::cos(angleRad);
            float sinA = std::sin(angleRad);
            QPointF dirRot(dir.x()*cosA - dir.y()*sinA, dir.x()*sinA + dir.y()*cosA);

            float baseForce = qBound(80.0f, len * 0.8f, 300.0f);
            float rnd = static_cast<float>(QRandomGenerator::global()->generateDouble());
            float forceMult = baseForce * (1.0f - forceNoisePct * rnd);
            QPointF forceVec = dirRot * forceMult;

            const float BOT_MAX_FORCE = 500.0f;
            float fLen = std::hypot(forceVec.x(), forceVec.y());
            if (fLen > BOT_MAX_FORCE) forceVec *= (BOT_MAX_FORCE / fLen);

            float score = logic.evaluateMove(bi, forceVec);
            candidates.append({ bi, forceVec, score });
        }
    }

    if (candidates.isEmpty()) {
        playerTurn = true;
        return;
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b){
        return a.score > b.score;
    });

    Candidate best = candidates.first();
    logic.shoot(best.checkerIndex, best.force);
    playerTurn = true;
}
