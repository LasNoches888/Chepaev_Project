#include "statsmanager.h"

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
    settings.setValue("whiteWins",  m_whiteWins);
    settings.setValue("blackWins",  m_blackWins);
    settings.setValue("draws",      m_draws);
}

void StatsManager::addGamePlayed()
{
    m_totalGames++;
    save();
}

void StatsManager::addWhiteWin()
{
    m_whiteWins++;
    addGamePlayed();
    save();
}

void StatsManager::addBlackWin()
{
    m_blackWins++;
    addGamePlayed();
    save();
}

void StatsManager::addDraw()
{
    m_draws++;
    addGamePlayed();
    save();
}
