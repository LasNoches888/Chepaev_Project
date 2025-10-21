#include "gamewidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <cmath>
#include <QRandomGenerator>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent),
    logic(),
    dragging(false),
    playerTurn(true),
    selectedChecker(-1),
    menuButtonHovered(false)
{
    // Сначала устанавливаем размеры
    setMinimumSize(800, 800);
    setMouseTracking(true); // Включить отслеживание мыши

    // Затем обновляем геометрию и инициализируем доску
    updateBoardGeometry();
    logic.initBoard();

    connect(&gameTimer, &QTimer::timeout, this, &GameWidget::onFrame);
    gameTimer.start(16);
}

void GameWidget::updateBoardGeometry()
{
    int width = this->width();
    int height = this->height();

    // Доска занимает 80% от минимальной стороны с минимальным размером 400
    int minSide = qMin(width, height);
    int boardSize = qMax(400, static_cast<int>(minSide * 0.8));

    // Центрируем доску
    int boardLeft = (width - boardSize) / 2;
    int boardTop = (height - boardSize) / 2;

    // Если размер доски значительно изменился, переинициализируем
    if (abs(logic.boardSize - boardSize) > 50.0f && !logic.isMoving()) {
        logic.boardLeft = boardLeft;
        logic.boardTop = boardTop;
        logic.boardSize = boardSize;
        logic.initBoard(); // Полная переинициализация
    } else {
        logic.boardLeft = boardLeft;
        logic.boardTop = boardTop;
        logic.boardSize = boardSize;
    }

    qDebug() << "Геометрия обновлена. Окно:" << width << "x" << height;
    qDebug() << "Доска:" << boardLeft << boardTop << boardSize;
}

void GameWidget::onFrame()
{
    logic.update(0.016f);

    // Проверяем окончание игры только когда шашки остановились
    if (!logic.isMoving()) {
        if (logic.checkGameOver()) {
            QString winner = logic.winner();
            if (winner != "none") {
                emit gameEnded(winner);
            }
        } else if (!playerTurn) {
            // Ход бота
            makeBotMove();
        }
    }

    update();
}

void GameWidget::makeBotMove()
{
    if (playerTurn || logic.isMoving() || logic.checkGameOver()) {
        return;
    }

    auto blackCheckers = logic.getBlackCheckers();
    if (blackCheckers.empty()) {
        playerTurn = true;
        return;
    }

    // Соберём белые шашки (цели) для эвристики
    auto whiteCheckers = logic.getWhiteCheckers();

    // Параметры случайности в зависимости от сложности
    float angleNoiseDeg = 0.0f;
    float forceNoisePct = 0.0f;
    int candidatesToConsider = 1;

    switch (difficulty) {
    case Easy:
        angleNoiseDeg = 40.0f;
        forceNoisePct = 0.5f;
        candidatesToConsider = qMax(1, static_cast<int>(blackCheckers.size()/2));
        break;
    case Medium:
        angleNoiseDeg = 18.0f;
        forceNoisePct = 0.25f;
        candidatesToConsider = qMax(1, static_cast<int>(blackCheckers.size()/3));
        break;
    case Hard:
        angleNoiseDeg = 6.0f;
        forceNoisePct = 0.12f;
        candidatesToConsider = qMax(1, static_cast<int>(blackCheckers.size()/4));
        break;
    }

    // Оценим для каждой черной шашки «очень простую» эвристику: цель — ближайшая белая
    struct Candidate { int idx; QPointF target; float score; QPointF force; };
    QVector<Candidate> candList;
    candList.reserve(blackCheckers.size());

    for (int bi : blackCheckers) {
        if (!logic.isCheckerAlive(bi)) continue;
        QPointF posB = logic.getCheckerPosition(bi);

        // Найдём ближайшую белую (если нет белых — стреляем в центр доски с небольшим разбросом)
        QPointF aimTarget;
        if (!whiteCheckers.empty()) {
            float bestDist = std::numeric_limits<float>::infinity();
            int bestW = -1;
            for (int wi : whiteCheckers) {
                if (!logic.isCheckerAlive(wi)) continue;
                QPointF posW = logic.getCheckerPosition(wi);
                float d = QLineF(posB, posW).length();
                if (d < bestDist) { bestDist = d; bestW = wi; }
            }
            if (bestW >= 0) {
                aimTarget = logic.getCheckerPosition(bestW);
                // Немного сместим цель в сторону между двумя ближайшими белыми (шанс выполнить комбинацию)
                if (whiteCheckers.size() >= 2) {
                    // найти вторую ближайшую
                    float secondDist = std::numeric_limits<float>::infinity();
                    int secondW = -1;
                    for (int wi : whiteCheckers) {
                        if (!logic.isCheckerAlive(wi)) continue;
                        QPointF posW = logic.getCheckerPosition(wi);
                        float d = QLineF(posB, posW).length();
                        if (d < secondDist && posW != aimTarget) { secondDist = d; secondW = wi; }
                    }
                    if (secondW >= 0) {
                        QPointF pos2 = logic.getCheckerPosition(secondW);
                        aimTarget = (aimTarget + pos2) / 2.0; // усреднить — попробовать комбинацию
                    }
                }
            } else {
                aimTarget = QPointF(logic.boardLeft + logic.boardSize/2.0f, logic.boardTop + logic.boardSize/2.0f);
            }
        } else {
            aimTarget = QPointF(logic.boardLeft + logic.boardSize/2.0f, logic.boardTop + logic.boardSize/2.0f);
        }

        // Базовый вектор силы: направлен от шашки к цели
        QPointF dir = aimTarget - posB;
        float dist = qMax(1.0f, std::sqrt(dir.x()*dir.x() + dir.y()*dir.y()));

        // Базовая сила в зависимости от расстояния (подогнать под физику игры)
        float baseForce = qBound(60.0f, dist * 0.8f, 300.0f);

        // Нормализуем направление
        QPointF unitDir = dir / dist;

        QPointF baseForceVec = unitDir * baseForce;

        // Оценочная функция: предпочтительнее близкие и направленные удары
        float score = 1000.0f / (dist + 1.0f);

        candList.append({bi, aimTarget, score, baseForceVec});
    }

    // Сортируем кандидатов по убыванию score
    std::sort(candList.begin(), candList.end(), [](const Candidate &a, const Candidate &b){
        return a.score > b.score;
    });

    // Рассматриваем top-N кандидатов, выбираем лучший с учётом случайности
    Candidate chosen = candList.first();
    int topN = qMin(candidatesToConsider, candList.size());
    float bestAdjustedScore = -1e9;
    for (int i = 0; i < topN; ++i) {
        Candidate c = candList[i];

        // Применим случайность в угле и силе (меньше шума на высоких уровнях сложности)
        float angleOffset = QRandomGenerator::global()->bounded(-angleNoiseDeg, angleNoiseDeg);
        float angleRad = angleOffset * 3.14159265f / 180.0f;

        QPointF f = c.force;
        float fx = f.x();
        float fy = f.y();
        // поворот вектора
        float rotatedX = fx * qCos(angleRad) - fy * qSin(angleRad);
        float rotatedY = fx * qSin(angleRad) + fy * qCos(angleRad);

        float noiseFactor = 1.0f + (QRandomGenerator::global()->bounded(-forceNoisePct, forceNoisePct));
        rotatedX *= noiseFactor;
        rotatedY *= noiseFactor;

        // скор скоринга: чем ближе направление к идеалу и чем больше сила — тем лучше
        float adjustedScore = c.score * (1.0f + 0.001f * qSqrt(rotatedX*rotatedX + rotatedY*rotatedY));
        // небольшая рандомизация предпочтений
        adjustedScore += QRandomGenerator::global()->bounded(-0.2f, 0.2f);

        if (adjustedScore > bestAdjustedScore) {
            bestAdjustedScore = adjustedScore;
            chosen = c;
            chosen.force = QPointF(rotatedX, rotatedY);
        }
    }

    // Выполняем выстрел выбранной шашкой
    logic.shoot(chosen.idx, chosen.force);

    // Возвращаем ход игроку
    playerTurn = true;
}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Фон
    p.fillRect(0, 0, width(), height(), QColor(60, 60, 80));

    // ОБНОВЛЯЕМ ГЕОМЕТРИЮ ПЕРЕД ОТРИСОВКОЙ
    updateBoardGeometry();

    // Рисуем доску и шашки
    logic.drawBoard(&p);

    // Линия прицеливания
    if (dragging && selectedChecker >= 0 && logic.isCheckerAlive(selectedChecker)) {
        QPointF checkerPos = logic.getCheckerPosition(selectedChecker);
        p.setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(checkerPos, currentMouse);

        // Стрелка на конце линии
        QPointF direction = currentMouse - checkerPos;
        float length = sqrt(direction.x()*direction.x() + direction.y()*direction.y());
        if (length > 0) {
            QPointF unitDir = direction / length;
            QPointF perpendicular(-unitDir.y(), unitDir.x());

            QPointF arrow1 = currentMouse - unitDir * 20 + perpendicular * 8;
            QPointF arrow2 = currentMouse - unitDir * 20 - perpendicular * 8;

            p.drawLine(currentMouse, arrow1);
            p.drawLine(currentMouse, arrow2);
        }
    }

    // Счет
    int whiteCount = logic.getWhiteCheckers().size();
    int blackCount = logic.getBlackCheckers().size();

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 14, QFont::Bold));

    // Фон для счета
    p.setBrush(QColor(0, 0, 0, 180));
    p.drawRect(10, 10, 200, 80);

    // Белые шашки
    p.setBrush(Qt::white);
    p.setPen(Qt::black);
    p.drawEllipse(20, 25, 20, 20);
    p.setPen(Qt::white);
    p.drawText(50, 40, QString("Белые: %1").arg(whiteCount));

    // Черные шашки
    p.setBrush(Qt::black);
    p.setPen(Qt::black);
    p.drawEllipse(20, 55, 20, 20);
    p.setPen(Qt::white);
    p.drawText(50, 70, QString("Черные: %1").arg(blackCount));

    // Кнопка "В меню" - делаем более заметной
    QRect menuButtonRect(width() - 130, 10, 120, 40);

    // Hover эффект для кнопки
    if (menuButtonHovered) {
        p.setBrush(QColor(255, 255, 255, 250));
    } else {
        p.setBrush(QColor(255, 255, 255, 200));
    }

    p.setPen(QPen(Qt::black, 2));
    p.drawRect(menuButtonRect);
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.drawText(menuButtonRect, Qt::AlignCenter, "В меню");

    // Чей ход
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 16, QFont::Bold));
    QString turnText = playerTurn ? "🎯 Ваш ход (белые)" : "🤖 Ход противника (черные)";
    QRect turnRect(width() / 2 - 150, height() - 50, 300, 30);
    p.drawText(turnRect, Qt::AlignCenter, turnText);
}

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }

    if (!playerTurn || logic.isMoving()) {
        return;
    }

    // ПРОВЕРКА КНОПКИ "В МЕНЮ"
    QRect menuButtonRect(width() - 130, 10, 120, 40);
    if (menuButtonRect.contains(e->pos())) {
        emit backToMenuClicked();
        return;
    }

    selectedChecker = -1;
    const float cell = logic.boardSize / 8.0f;
    const float radius = cell * 0.4f;

    const auto& checkers = logic.getCheckers();
    for (int i = 0; i < checkers.size(); ++i) {
        const auto& c = checkers[i];
        if (!c->alive || c->color != Qt::white) continue;

        float dist = sqrt(pow(e->pos().x() - c->pos.x(), 2) + pow(e->pos().y() - c->pos.y(), 2));
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
    // Проверка hover для кнопки
    QRect menuButtonRect(width() - 130, 10, 120, 40);
    bool wasHovered = menuButtonHovered;
    menuButtonHovered = menuButtonRect.contains(e->pos());

    if (wasHovered != menuButtonHovered) {
        update(); // Перерисовать если состояние изменилось
    }

    if (dragging && selectedChecker >= 0) {
        currentMouse = e->pos();
        update();
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }

    if (dragging && selectedChecker >= 0) {
        dragging = false;

        QPointF checkerPos = logic.getCheckerPosition(selectedChecker);
        QPointF direction = checkerPos - e->pos();
        float forceLength = sqrt(direction.x()*direction.x() + direction.y()*direction.y());

        if (forceLength > 10) {
            QPointF force = direction * 2.0f;
            logic.shoot(selectedChecker, force);
            playerTurn = false; // Передаем ход боту
        }

        selectedChecker = -1;
        update();
    }
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBoardGeometry();
    update();
}

QSize GameWidget::sizeHint() const
{
    return QSize(800, 800);
}
