#include "gamelogic.h"
#include <cmath>
#include <QDebug>

GameLogic::GameLogic()
    : boardLeft(100), boardTop(100), boardSize(600),
    winnerColor(""), gameOver(false), botDifficulty(Medium)
{
}

float GameLogic::length(const QPointF &v) const {
    return std::sqrt(v.x()*v.x() + v.y()*v.y());
}

void GameLogic::initBoard()
{
    checkers.clear();
    initialPositions.clear(); // Очищаем начальные позиции

    const float cell = boardSize / 8.0f;

    qDebug() << "=== ИНИЦИАЛИЗАЦИЯ ДОСКИ ===";
    qDebug() << "Размер доски:" << boardSize;
    qDebug() << "Позиция доски:" << boardLeft << boardTop;
    qDebug() << "Размер ячейки:" << cell;

    // БЕЛЫЕ ШАШКИ (нижние 2 ряда для игрока)
    int checkerCount = 0;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 4; ++col) {
            // Правильное расположение в шахматном порядке
            int actualRow = 6 + row; // 6 и 7 ряды (нижние)
            int actualCol = col * 2 + ((row % 2 == 0) ? 1 : 0);

            float x = boardLeft + cell * (actualCol + 0.5f);
            float y = boardTop + cell * (actualRow + 0.5f);

            QPointF pos(x, y);
            checkers.push_back(std::make_shared<Checker>(pos, Qt::white));
            // Сохраняем относительную позицию (0-1 относительно доски)
            float relX = (x - boardLeft) / boardSize;
            float relY = (y - boardTop) / boardSize;
            initialPositions.push_back(QPointF(relX, relY));

            checkerCount++;
            qDebug() << "Белая шашка" << checkerCount << "позиция:" << pos << "относительная:" << relX << relY;
        }
    }

    // ЧЕРНЫЕ ШАШКИ (верхние 2 ряда для бота)
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 4; ++col) {
            // Правильное расположение в шахматном порядке
            int actualRow = row; // 0 и 1 ряды (верхние)
            int actualCol = col * 2 + ((row % 2 == 0) ? 1 : 0);

            float x = boardLeft + cell * (actualCol + 0.5f);
            float y = boardTop + cell * (actualRow + 0.5f);

            QPointF pos(x, y);
            checkers.push_back(std::make_shared<Checker>(pos, Qt::black));
            // Сохраняем относительную позицию (0-1 относительно доски)
            float relX = (x - boardLeft) / boardSize;
            float relY = (y - boardTop) / boardSize;
            initialPositions.push_back(QPointF(relX, relY));

            checkerCount++;
            qDebug() << "Черная шашка" << checkerCount << "позиция:" << pos << "относительная:" << relX << relY;
        }
    }

    gameOver = false;
    winnerColor = "";

    qDebug() << "=== ДОСКА ИНИЦИАЛИЗИРОВАНА ===";
    qDebug() << "Всего шашек:" << checkers.size();
    qDebug() << "Белых:" << getWhiteCheckers().size()
             << "Черных:" << getBlackCheckers().size();
}

// НОВЫЙ МЕТОД: обновление позиций шашек при изменении размера доски
void GameLogic::updateCheckerPositions()
{
    // Обновляем только если шашки не двигаются
    if (isMoving()) return;

    // используем initialPositions, boardLeft/boardTop/boardSize напрямую
    for (int i = 0; i < checkers.size(); ++i) {
        if (!checkers[i]->alive) continue;

        // Восстанавливаем позицию из относительных координат
        float x = boardLeft + initialPositions[i].x() * boardSize;
        float y = boardTop + initialPositions[i].y() * boardSize;

        checkers[i]->pos = QPointF(x, y);
        checkers[i]->vel = QPointF(0, 0); // Сбрасываем скорость
    }

    qDebug() << "Позиции шашек обновлены под новый размер доски:" << boardSize;
}

void GameLogic::drawBoard(QPainter *p)
{
    const float cell = boardSize / 8.0f;

    // Рисуем клетки доски
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QRectF cellRect(
                boardLeft + col * cell,
                boardTop + row * cell,
                cell,
                cell
                );

            // Чередуем цвета клеток
            if ((row + col) % 2 == 0) {
                p->fillRect(cellRect, QColor(240, 217, 181)); // Светлые клетки
            } else {
                p->fillRect(cellRect, QColor(181, 136, 99));  // Темные клетки
            }
        }
    }

    // Рамка доски
    p->setPen(QPen(Qt::black, 3));
    p->drawRect(boardLeft, boardTop, boardSize, boardSize);

    // Рисуем шашки
    const float radius = cell * 0.4f;
    for (auto &c : checkers) {
        if (!c->alive) continue;

        // Основной круг шашки
        p->setBrush(c->color);
        p->setPen(QPen(Qt::black, 2));
        p->drawEllipse(c->pos, radius, radius);

        // Добавляем ободок для лучшего визуального эффекта
        if (c->color == Qt::white) {
            p->setPen(QPen(QColor(200, 200, 200), 1));
            p->drawEllipse(c->pos, radius - 2, radius - 2);
        } else {
            p->setPen(QPen(QColor(50, 50, 50), 1));
            p->drawEllipse(c->pos, radius - 2, radius - 2);
        }
    }
}

// ... остальные методы без изменений ...

void GameLogic::update(float dt)
{
    if (gameOver) return;

    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    // Применяем физику движения и помечаем шашки как неактивные, как только центр шашки
    // полностью ушёл за пределы доски (т.е. шашка полностью покинула игровую область).
    for (auto &c : checkers) {
        if (!c->alive) continue;

        // Применяем трение
        c->vel *= 0.98f;

        // Обновляем позицию
        c->pos += c->vel * dt;

        // Пометка неактивной, как только шашка полностью покинула игровое поле
        // (центр +/− радиус за пределами границ).
        bool leftOfBoard   = (c->pos.x() + radius) < boardLeft;
        bool rightOfBoard  = (c->pos.x() - radius) > (boardLeft + boardSize);
        bool aboveBoard    = (c->pos.y() + radius) < boardTop;
        bool belowBoard    = (c->pos.y() - radius) > (boardTop + boardSize);

        if (leftOfBoard || rightOfBoard || aboveBoard || belowBoard) {
            c->alive = false;
            c->vel = QPointF(0,0);
            qDebug() << "Шашка полностью покинула поле и помечена как неактивная. Цвет:"
                     << (c->color == Qt::white ? "белая" : "черная") << "поз:" << c->pos;
            continue;
        }

        // Небольшие "подтягивания" больше не выполняются - шашки могут выезжать.
        // При этом, если шашка немного ушла за край (но ещё не полностью),
        // она остаётся активной и может вернуться обратно в результате столкновений.
    }

    // Обрабатываем столкновения (только между живыми шашками)
    handleCollisions();
}

void GameLogic::handleCollisions()
{
    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    for (int i = 0; i < checkers.size(); ++i) {
        auto &a = checkers[i];
        if (!a->alive) continue;

        for (int j = i + 1; j < checkers.size(); ++j) {
            auto &b = checkers[j];
            if (!b->alive) continue;

            QPointF diff = b->pos - a->pos;
            float dist = length(diff);
            if (dist > 0 && dist < 2 * radius) {
                // Столкновение
                QPointF n = diff / dist;
                float overlap = 2 * radius - dist;

                // Раздвигаем шашки
                a->pos -= n * overlap / 2.0f;
                b->pos += n * overlap / 2.0f;

                // Обмен скоростями
                QPointF relativeVelocity = b->vel - a->vel;
                float velocityAlongNormal = relativeVelocity.x() * n.x() + relativeVelocity.y() * n.y();

                if (velocityAlongNormal > 0) continue;

                float restitution = 0.8f;
                float impulse = -(1.0f + restitution) * velocityAlongNormal / 2.0f;

                QPointF impulseVector = n * impulse;
                a->vel -= impulseVector;
                b->vel += impulseVector;
            }
        }
    }
}

BotMove GameLogic::findBestMove(QColor botColor) const
{
    QVector<BotMove> possibleMoves;

    // Получаем шашки бота
    QVector<int> botCheckers = (botColor == Qt::black) ? getBlackCheckers() : getWhiteCheckers();

    if (botCheckers.isEmpty()) return {-1, QPointF(0,0), -1000};

    // Подбираем параметры с уклоном по сложности: чем сложнее — тем уже область поиска углов
    float angleSpreadDeg = 45.0f;
    int powerMin = 100;
    int powerMax = 300;
    switch (botDifficulty) {
    case Easy:   angleSpreadDeg = 90.0f; powerMin = 90;  powerMax = 220; break;
    case Medium: angleSpreadDeg = 45.0f; powerMin = 120; powerMax = 320; break;
    case Hard:   angleSpreadDeg = 22.0f; powerMin = 140; powerMax = 400; break;
    default: break;
    }

    // Для каждой шашки бота: вычисляем направление на ближайшего врага и пробуем углы вокруг него
    for (int checkerIndex : botCheckers) {
        QPointF startPos = getCheckerPosition(checkerIndex);

        // находим ближайшую вражескую шашку как цель
        QColor enemyColor = (botColor == Qt::black) ? Qt::white : Qt::black;
        float bestD = 1e9f;
        QPointF targetPos = startPos + QPointF(0, boardSize * 0.2f); // если врагов нет — двигаться "вперёд" по Y
        for (int i = 0; i < checkers.size(); ++i) {
            if (!checkers[i]->alive || checkers[i]->color != enemyColor) continue;
            QPointF p = checkers[i]->pos;
            float d = length(p - startPos);
            if (d < bestD) { bestD = d; targetPos = p; }
        }

        QPointF dir = targetPos - startPos;
        float dirLen = length(dir);
        if (dirLen > 0.0001f) dir /= dirLen;
        else dir = QPointF(0.0f, (botColor == Qt::black) ? 1.0f : -1.0f);

        // базовый угол в градусах
        float baseAngle = std::atan2(dir.y(), dir.x()) * 180.0f / 3.14159265f;

        // пробуем несколько углов вокруг базового направления
        int angleSteps = 8;
        for (int s = -angleSteps; s <= angleSteps; ++s) {
            float frac = (float)s / (float)angleSteps;
            float angleDeg = baseAngle + frac * angleSpreadDeg;
            float rad = angleDeg * 3.14159265f / 180.0f;

            // перебор силы
            for (int power = powerMin; power <= powerMax; power += (powerMax - powerMin) / 3 + 1) {
                QPointF force(std::cos(rad) * power, std::sin(rad) * power);

                float score = evaluateMove(checkerIndex, force);

                // Доп. бонусы/штрафы уже считаются в evaluateMove — тут можно добавить ещё эвристики при желании

                possibleMoves.push_back({checkerIndex, force, score});
            }
        }
    }

    // Выбираем лучший ход
    if (possibleMoves.isEmpty()) {
        return {-1, QPointF(0, 0), -1000};
    }

    BotMove bestMove = possibleMoves.first();
    for (const BotMove &move : possibleMoves) {
        if (move.score > bestMove.score) {
            bestMove = move;
        }
    }

    qDebug() << "Лучший ход бота: шашка" << bestMove.checkerIndex
             << "сила:" << bestMove.force << "очки:" << bestMove.score;

    return bestMove;
}

void GameLogic::shoot(int checkerIndex, const QPointF &force)
{
    if (gameOver || checkerIndex < 0 || checkerIndex >= checkers.size()) return;

    auto &c = checkers[checkerIndex];
    if (c->alive) {
        c->vel = force;
        qDebug() << "Выстрел по шашке" << checkerIndex << "сила:" << force;
    }
}

bool GameLogic::checkGameOver() const
{
    int whiteAlive = 0;
    int blackAlive = 0;

    for (auto &c : checkers) {
        if (c->alive) {
            if (c->color == Qt::white) whiteAlive++;
            else blackAlive++;
        }
    }

    qDebug() << "Проверка окончания игры. Белые:" << whiteAlive << "Черные:" << blackAlive;

    // Игра заканчивается только если у одной из сторон не осталось шашек
    bool gameEnded = (whiteAlive == 0 || blackAlive == 0);

    if (gameEnded) {
        qDebug() << "=== ИГРА ОКОНЧЕНА ===";
        qDebug() << "Белых осталось:" << whiteAlive;
        qDebug() << "Черных осталось:" << blackAlive;
    }

    return gameEnded;
}

QString GameLogic::winner() const
{
    int whiteAlive = 0;
    int blackAlive = 0;
    for (auto &c : checkers) {
        if (c->alive) {
            if (c->color == Qt::white) whiteAlive++;
            else blackAlive++;
        }
    }

    qDebug() << "Определение победителя. Белые:" << whiteAlive << "Черные:" << blackAlive;

    if (whiteAlive == 0 && blackAlive == 0) {
        qDebug() << "НИЧЬЯ - все шашки выбиты";
        return "draw";
    }
    if (whiteAlive == 0) {
        qDebug() << "ПОБЕДА ЧЕРНЫХ";
        return "black";
    }
    if (blackAlive == 0) {
        qDebug() << "ПОБЕДА БЕЛЫХ";
        return "white";
    }

    qDebug() << "Игра продолжается";
    return "none";
}

bool GameLogic::isMoving() const
{
    for (auto &c : checkers) {
        if (c->alive && length(c->vel) > 0.5f) {
            return true;
        }
    }
    return false;
}

int GameLogic::getCheckerAtPosition(const QPointF &pos) const
{
    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    for (int i = 0; i < checkers.size(); ++i) {
        auto &c = checkers[i];
        if (!c->alive) continue;

        float dist = length(pos - c->pos);
        if (dist <= radius) {
            return i;
        }
    }
    return -1;
}

QColor GameLogic::getCheckerColor(int index) const
{
    if (index >= 0 && index < checkers.size()) {
        return checkers[index]->color;
    }
    return Qt::transparent;
}

QPointF GameLogic::getCheckerPosition(int index) const
{
    if (index >= 0 && index < checkers.size()) {
        return checkers[index]->pos;
    }
    return QPointF(0, 0);
}

QVector<int> GameLogic::getBlackCheckers() const
{
    QVector<int> result;
    for (int i = 0; i < checkers.size(); ++i) {
        if (checkers[i]->alive && checkers[i]->color == Qt::black) {
            result.push_back(i);
        }
    }
    return result;
}

QVector<int> GameLogic::getWhiteCheckers() const
{
    QVector<int> result;
    for (int i = 0; i < checkers.size(); ++i) {
        if (checkers[i]->alive && checkers[i]->color == Qt::white) {
            result.push_back(i);
        }
    }
    return result;
}

float GameLogic::evaluateMove(int checkerIndex, const QPointF &force) const
{
    if (checkerIndex < 0 || checkerIndex >= checkers.size()) return -1000;

    auto checker = checkers[checkerIndex];
    if (!checker->alive) return -1000;

    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    // Базовая ценность силы — но не делаем силу единственным критерием
    float score = 0.2f * length(force);

    // Предсказываем позицию через небольшой промежуток времени (чтобы понять, попадём ли в противника)
    QPointF predictedPos = predictPosition(checker->pos, force, 0.8f);

    // Штраф за вылет за пределы (как только шашка полностью покинет поле, оцениваем это плохо)
    bool willLeaveLeft   = (predictedPos.x() + radius) < boardLeft;
    bool willLeaveRight  = (predictedPos.x() - radius) > (boardLeft + boardSize);
    bool willLeaveAbove  = (predictedPos.y() + radius) < boardTop;
    bool willLeaveBelow  = (predictedPos.y() - radius) > (boardTop + boardSize);

    if (willLeaveLeft || willLeaveRight || willLeaveAbove || willLeaveBelow) {
        score -= 250.0f; // существенный штраф — бот должен избегать потери шашки
    } else {
        score += 20.0f; // бонус за то, что шашка останется на доске
    }

    // Бонус за потенциальный удар по вражеской шашке
    QColor enemyColor = (checker->color == Qt::black) ? Qt::white : Qt::black;
    float bestDist = 1e9f;
    int potentialHits = 0;
    for (int i = 0; i < checkers.size(); ++i) {
        if (!checkers[i]->alive || checkers[i]->color != enemyColor) continue;
        QPointF enemyPos = checkers[i]->pos;
        float d = length(predictedPos - enemyPos);
        if (d < bestDist) bestDist = d;
        if (d < radius * 1.4f) potentialHits++;
    }
    if (potentialHits > 0) {
        // сильный бонус за возможность попасть в противника
        score += 220.0f + potentialHits * 80.0f;
    } else {
        // если близко к вражеской шашке — небольшой бонус
        if (bestDist < radius * 4.0f) score += 60.0f;
    }

    // Небольшая штрафная поправка за слишком "сильную" силу, если это не ведёт к атаке
    if (potentialHits == 0) {
        float fmag = length(force);
        if (fmag > 350.0f) score -= (fmag - 350.0f) * 0.25f;
    }

    // Возвращаем комбинированную оценку
    return score;
}

QPointF GameLogic::predictPosition(const QPointF &startPos, const QPointF &startVel, float time) const
{
    QPointF pos = startPos;
    QPointF vel = startVel;

    // Простая имитация движения с трением; если покинет поле — прекращаем
    const float dt = 0.05f;
    for (float t = 0; t < time; t += dt) {
        vel *= 0.99f;
        pos += vel * dt;

        if ((pos.x() + (boardSize/8.0f * 0.4f)) < boardLeft ||
            (pos.x() - (boardSize/8.0f * 0.4f)) > (boardLeft + boardSize) ||
            (pos.y() + (boardSize/8.0f * 0.4f)) < boardTop ||
            (pos.y() - (boardSize/8.0f * 0.4f)) > (boardTop + boardSize)) {
            break;
        }
    }

    return pos;
}
