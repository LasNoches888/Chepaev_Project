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

// НОВЫЙ МЕТОД: масштабирование позиций при смене геометрии доски
void GameLogic::rescaleBoard(float oldLeft, float oldTop, float oldSize,
                             float newLeft, float newTop, float newSize)
{
    if (oldSize <= 0.0f) {
        // если старый размер некорректен — просто обновляем начальные относительные позиции по текущему состоянию
        for (int i = 0; i < checkers.size(); ++i) {
            if (!checkers[i]) continue;
            QPointF p = checkers[i]->pos;
            float relX = (p.x() - newLeft) / newSize;
            float relY = (p.y() - newTop) / newSize;
            if (i < initialPositions.size()) initialPositions[i] = QPointF(relX, relY);
            else initialPositions.push_back(QPointF(relX, relY));
        }
        boardLeft = newLeft;
        boardTop = newTop;
        boardSize = newSize;
        return;
    }

    float scale = newSize / oldSize;
    float dx = newLeft - oldLeft * scale;
    float dy = newTop - oldTop * scale;

    // Вместо грубого перерасчёта относительно старой начальной сетки, мы масштабируем текущие позиции
    for (int i = 0; i < checkers.size(); ++i) {
        auto &c = checkers[i];
        if (!c) continue;
        // относительные по старой геометрии
        float relX_old = (c->pos.x() - oldLeft) / oldSize;
        float relY_old = (c->pos.y() - oldTop) / oldSize;

        // новый абсолютный
        float newX = newLeft + relX_old * newSize;
        float newY = newTop + relY_old * newSize;

        c->pos = QPointF(newX, newY);
        // скорости тоже масштабируем (чтобы эффект перемещения оставался согласованным)
        c->vel *= scale;

        // корректируем сохранённые относительные начальные позиции
        if (i < initialPositions.size()) {
            initialPositions[i] = QPointF((c->pos.x() - newLeft) / newSize,
                                          (c->pos.y() - newTop) / newSize);
        } else {
            initialPositions.push_back(QPointF((c->pos.x() - newLeft) / newSize,
                                               (c->pos.y() - newTop) / newSize));
        }
    }

    boardLeft = newLeft;
    boardTop = newTop;
    boardSize = newSize;

    qDebug() << "rescaleBoard: oldSize=" << oldSize << " newSize=" << newSize << " scale=" << scale;
}

// НОВЫЙ: обновление позиций шашек при изменении размера доски
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

    // Целевые шашки врага
    QVector<int> enemyCheckers = (botColor == Qt::black) ? getWhiteCheckers() : getBlackCheckers();

    if (botCheckers.isEmpty()) {
        return {-1, QPointF(0,0), -1000};
    }

    // Параметры поиска в зависимости от сложности
    int angleStep = 30;
    int powerSamples = 3;
    switch (botDifficulty) {
    case Easy: angleStep = 60; powerSamples = 1; break;
    case Medium: angleStep = 30; powerSamples = 3; break;
    case Hard: angleStep = 10; powerSamples = 6; break;
    default: break;
    }

    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    // Для каждого бот-чекера пробуем ходы, прицеливаясь в ближайшую вражескую шашку (если есть)
    for (int bi : botCheckers) {
        if (!isCheckerAlive(bi)) continue;
        QPointF startPos = getCheckerPosition(bi);

        // Выбираем несколько приоритетных целей (ближайшие)
        QVector<int> targets = enemyCheckers;
        std::sort(targets.begin(), targets.end(), [&](int a, int b){
            float da = length(getCheckerPosition(a) - startPos);
            float db = length(getCheckerPosition(b) - startPos);
            return da < db;
        });

        if (targets.isEmpty()) {
            // если целей нет — стреляем в центр
            targets.push_back(bi);
        }

        // ограничим число целей, чтобы не гоняться за всеми
        int maxTargets = qMin(3, targets.size());
        for (int ti = 0; ti < maxTargets; ++ti) {
            int targetIndex = targets[ti];
            QPointF targetPos = getCheckerPosition(targetIndex);

            QPointF idealDir = targetPos - startPos;
            float dist = length(idealDir);
            if (dist > 0) idealDir /= dist;
            else idealDir = QPointF(0, 1);

            // базовая сила — пропорциональна расстоянию
            float basePower = qBound(80.0f, dist * 0.7f, 500.0f);

            // пробуем углы вокруг идеальной линии, и несколько сил
            int halfRangeDeg = 40;
            for (int a = -halfRangeDeg; a <= halfRangeDeg; a += angleStep) {
                float rad = a * 3.14159265f / 180.0f;
                float cosA = std::cos(rad);
                float sinA = std::sin(rad);
                QPointF dirRot(idealDir.x()*cosA - idealDir.y()*sinA,
                               idealDir.x()*sinA + idealDir.y()*cosA);

                for (int p = 0; p < powerSamples; ++p) {
                    float frac = (powerSamples > 1) ? (float)p / (powerSamples - 1) : 0.5f;
                    float powerMult = 0.85f + 0.3f * (frac - 0.5f); // немного варьируем силу
                    QPointF force = dirRot * (basePower * powerMult);

                    // предсказываем позицию через короткое время с учётом трения
                    QPointF predicted = predictPosition(startPos, force, 2.0f);

                    // оценка: расстояние до целевой шашки + бонусы за попадения по другим
                    float score = length(force); // базовый приоритет — сила (чтобы бить)
                    // штраф за вылет из доски
                    if (predicted.x() < boardLeft || predicted.x() > boardLeft + boardSize ||
                        predicted.y() < boardTop || predicted.y() > boardTop + boardSize) {
                        score -= 150;
                    } else {
                        score += 20; // небольшая награда за оставание на доске
                    }

                    // Бонусы за близость к цели и потенциальные попадания по другим
                    int hitCandidates = 0;
                    for (int ei = 0; ei < checkers.size(); ++ei) {
                        if (!checkers[ei]->alive) continue;
                        if (checkers[ei]->color == botColor) continue; // свои — не считаем
                        QPointF ep = checkers[ei]->pos;
                        float d = length(predicted - ep);
                        if (d < radius * 1.6f) {
                            score += 80.0f; // сильный бонус за потенциальный прямой удар
                            hitCandidates++;
                        } else if (d < radius * 3.0f) {
                            score += 18.0f; // небольшой бонус за близкую атаку
                        }
                    }

                    // бонус за многопопадание
                    if (hitCandidates > 1) score += hitCandidates * 25.0f;

                    possibleMoves.push_back({bi, force, score});
                }
            }
        }
    }

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
