#include "gamelogic.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

GameLogic::GameLogic()
    : whitePlayer("Белые", false),
    blackPlayer("Чёрные (бот)", true),
    boardLeft(50),
    boardTop(50),
    boardSize(700)
{
}

float GameLogic::length(const QPointF &v) const {
    return std::sqrt(v.x() * v.x() + v.y() * v.y());
}

void GameLogic::initBoard() {
    checkers.clear();
    const float cell = boardSize / 8.0f;

    // создаём по 6 шашек каждой стороны в два ряда
    for (int i = 0; i < 6; ++i) {
        QPointF p1(boardLeft + cell * (i + 1), boardTop + cell * 1.0);
        QPointF p2(boardLeft + cell * (i + 1), boardTop + cell * 6.0);
        checkers.push_back(std::make_shared<Checker>(p1, Qt::white));
        checkers.push_back(std::make_shared<Checker>(p2, Qt::black));
    }

    whitePlayer.score = 0;
    blackPlayer.score = 0;
}

void GameLogic::handleCollisions() {
    const float cell = boardSize / 8.0f;
    const float radius = cell * 0.4f;

    // столкновения между шашками (простая физика)
    for (size_t i = 0; i < checkers.size(); ++i) {
        for (size_t j = i + 1; j < checkers.size(); ++j) {
            auto &a = checkers[i];
            auto &b = checkers[j];
            if (!a->alive || !b->alive) continue;

            QPointF diff = b->pos - a->pos;
            float dist = length(diff);
            if (dist > 0 && dist < 2 * radius) {
                QPointF n = diff / dist;
                float overlap = 2 * radius - dist;
                a->pos -= n * (overlap / 2.0);
                b->pos += n * (overlap / 2.0);

                // обмен скоростями
                QPointF va = a->vel;
                a->vel = b->vel;
                b->vel = va;
            }
        }
    }

    // удаление шашек, вылетевших за пределы
    for (auto &c : checkers) {
        if (!c->alive) continue;
        if (c->pos.x() < boardLeft || c->pos.x() > boardLeft + boardSize ||
            c->pos.y() < boardTop || c->pos.y() > boardTop + boardSize) {
            c->alive = false;
            if (c->color == Qt::white)
                blackPlayer.score++;
            else
                whitePlayer.score++;
        }
    }
}

void GameLogic::update(float dt)
{
    for (auto &c : checkers) {
        if (!c->alive) continue;
        c->pos += c->vel * dt;
        c->vel *= 0.98f; // трение
    }

    handleCollisions();
}

void GameLogic::botMakeMove()
{
    // собираем живые шашки
    std::vector<std::shared_ptr<Checker>> botPieces;
    std::vector<std::shared_ptr<Checker>> whitePieces;
    for (auto &c : checkers) {
        if (!c->alive) continue;
        if (c->color == Qt::black)
            botPieces.push_back(c);
        else
            whitePieces.push_back(c);
    }

    if (botPieces.empty() || whitePieces.empty()) return;

    // выбираем шашку бота
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> botDist(0, botPieces.size() - 1);
    auto botChecker = botPieces[botDist(gen)];

    // ищем ближайшую белую шашку
    std::shared_ptr<Checker> target = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (auto &w : whitePieces) {
        float d = length(w->pos - botChecker->pos);
        if (d < minDist) {
            minDist = d;
            target = w;
        }
    }

    if (!target) return;

    // направление на цель
    QPointF dir = target->pos - botChecker->pos;
    float len = length(dir);
    if (len < 1e-3f) return;

    dir /= len;

    // сила удара
    float impulsePower = 300.0f + (float)(rand() % 150);
    botChecker->vel = dir * impulsePower * 0.02f;
}
