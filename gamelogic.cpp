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

    for (auto &c : checkers) {
        if (!c->alive) continue;

        // Применяем трение
        c->vel *= 0.98f;

        // Обновляем позицию
        c->pos += c->vel * dt;

        // Жёсткая проверка выхода за границы доски: если вышли за пределы — помечаем как неактивную
        if (c->pos.x() < boardLeft || c->pos.x() > boardLeft + boardSize ||
            c->pos.y() < boardTop  || c->pos.y() > boardTop  + boardSize)
        {
            c->alive = false;
            c->vel = QPointF(0,0); // обнуляем скорость, чтобы объект не двигался дальше
            qDebug() << "Шашка вылетела за пределы и помечена как неактивная. Цвет:"
                     << (c->color == Qt::white ? "белая" : "черная") << "поз:" << c->pos;
            continue;
        }
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

    // Проверяем все возможные ходы
    for (int checkerIndex : botCheckers) {
        // Проверяем несколько направлений с разной силой
        for (int angle = 0; angle < 360; angle += 30) {
            for (int power = 100; power <= 300; power += 100) {
                float rad = angle * 3.14159f / 180.0f;
                QPointF force(cos(rad) * power, sin(rad) * power);

                float score = evaluateMove(checkerIndex, force);

                // Бонус за атаку вражеских шашек
                QPointF predictedPos = predictPosition(
                    getCheckerPosition(checkerIndex),
                    force,
                    2.0f
                    );

                // Проверяем столкновения с вражескими шашками
                QColor enemyColor = (botColor == Qt::black) ? Qt::white : Qt::black;
                for (int i = 0; i < checkers.size(); ++i) {
                    if (checkers[i]->alive && checkers[i]->color == enemyColor) {
                        QPointF enemyPos = checkers[i]->pos;
                        float dist = length(predictedPos - enemyPos);
                        const float cell = boardSize / 8.0f;
                        const float radius = cell * 0.4f;

                        if (dist < radius * 3) {
                            score += 50; // Бонус за атаку врага
                        }
                    }
                }

                // Штраф за вылет за пределы
                if (predictedPos.x() < boardLeft || predictedPos.x() > boardLeft + boardSize ||
                    predictedPos.y() < boardTop || predictedPos.y() > boardTop + boardSize) {
                    score -= 100;
                }

                // Бонус за оставание в пределах доски
                if (predictedPos.x() >= boardLeft && predictedPos.x() <= boardLeft + boardSize &&
                    predictedPos.y() >= boardTop && predictedPos.y() <= boardTop + boardSize) {
                    score += 30;
                }

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

    float score = length(force);

    // Штраф за вылет за пределы
    QPointF predictedPos = checker->pos + force * 0.1f;
    if (predictedPos.x() < boardLeft || predictedPos.x() > boardLeft + boardSize ||
        predictedPos.y() < boardTop || predictedPos.y() > boardTop + boardSize) {
        score -= 100;
    }

    return score;
}

QPointF GameLogic::predictPosition(const QPointF &startPos, const QPointF &startVel, float time) const
{
    QPointF pos = startPos;
    QPointF vel = startVel;

    for (float t = 0; t < time; t += 0.1f) {
        vel *= 0.99f;
        pos += vel * 0.1f;

        if (pos.x() < boardLeft || pos.x() > boardLeft + boardSize ||
            pos.y() < boardTop || pos.y() > boardTop + boardSize) {
            break;
        }
    }

    return pos;
}
