#include "statsmanager.h"
#include <QDebug>

StatsManager::StatsManager(QObject *parent)
    : QObject(parent),
    m_totalGames(0),
    m_whiteWins(0),
    m_blackWins(0),
    m_draws(0)
{
    load();
}

void StatsManager::load()
{
    QSettings settings("ChepaevGame", "Stats");
    m_totalGames = settings.value("totalGames", 0).toInt();
    m_whiteWins  = settings.value("whiteWins", 0).toInt();
    m_blackWins  = settings.value("blackWins", 0).toInt();
    m_draws      = settings.value("draws", 0).toInt();
}

void StatsManager::save() const
{
    QSettings settings("ChepaevGame", "Stats");
    settings.setValue("totalGames", m_totalGames);
    settings.setValue("whiteWins", m_whiteWins);
    settings.setValue("blackWins", m_blackWins);
    settings.setValue("draws", m_draws);
}

void StatsManager::addGameResult(const QString &winner)
{
    m_totalGames++;

    if (winner == "white") {
        m_whiteWins++;
    } else if (winner == "black") {
        m_blackWins++;
    } else if (winner == "draw") {
        m_draws++;
    }

    save();

    qDebug() << "Статистика обновлена:";
    qDebug() << "Всего игр:" << m_totalGames;
    qDebug() << "Побед белых:" << m_whiteWins;
    qDebug() << "Побед черных:" << m_blackWins;
    qDebug() << "Ничьих:" << m_draws;
}

void StatsManager::reset()
{
    m_totalGames = 0;
    m_whiteWins = 0;
    m_blackWins = 0;
    m_draws = 0;
    save();
}

double StatsManager::whiteWinPercent() const
{
    if (m_totalGames == 0) return 0.0;
    return (static_cast<double>(m_whiteWins) / m_totalGames) * 100.0;
}

double StatsManager::blackWinPercent() const
{
    if (m_totalGames == 0) return 0.0;
    return (static_cast<double>(m_blackWins) / m_totalGames) * 100.0;
}

double StatsManager::drawPercent() const
{
    if (m_totalGames == 0) return 0.0;
    return (static_cast<double>(m_draws) / m_totalGames) * 100.0;
}
