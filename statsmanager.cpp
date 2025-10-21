#include "statsmanager.h"
#include <QDebug>

StatsManager::StatsManager(QObject *parent)
    : QObject(parent),
    m_totalGames(0),
    m_whiteWins(0),
    m_blackWins(0),
    m_draws(0),
    m_longestWinStreak(0),
    m_currentWinStreak(0),
    m_lastWinner("")
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

    m_longestWinStreak = settings.value("longestWinStreak", 0).toInt();
    m_currentWinStreak = settings.value("currentWinStreak", 0).toInt();
    m_lastWinner       = settings.value("lastWinner", "").toString();
}

void StatsManager::save() const
{
    QSettings settings("ChepaevGame", "Stats");
    settings.setValue("totalGames", m_totalGames);
    settings.setValue("whiteWins", m_whiteWins);
    settings.setValue("blackWins", m_blackWins);
    settings.setValue("draws", m_draws);

    settings.setValue("longestWinStreak", m_longestWinStreak);
    settings.setValue("currentWinStreak", m_currentWinStreak);
    settings.setValue("lastWinner", m_lastWinner);
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

    // Обновление полос побед (streaks)
    if (winner == "draw") {
        // ничья — сбрасываем текущую серию
        m_currentWinStreak = 0;
        m_lastWinner.clear();
    } else {
        if (winner == m_lastWinner && !m_lastWinner.isEmpty()) {
            m_currentWinStreak++;
        } else {
            m_currentWinStreak = 1;
            m_lastWinner = winner;
        }
        if (m_currentWinStreak > m_longestWinStreak) {
            m_longestWinStreak = m_currentWinStreak;
        }
    }

    save();

    qDebug() << "Статистика обновлена:";
    qDebug() << "Всего игр:" << m_totalGames;
    qDebug() << "Побед белых:" << m_whiteWins;
    qDebug() << "Побед черных:" << m_blackWins;
    qDebug() << "Ничьих:" << m_draws;
    qDebug() << "Текущая серия побед:" << m_currentWinStreak << "Последний победитель:" << m_lastWinner;
    qDebug() << "Максимальная серия побед:" << m_longestWinStreak;
}

void StatsManager::reset()
{
    m_totalGames = 0;
    m_whiteWins = 0;
    m_blackWins = 0;
    m_draws = 0;

    m_longestWinStreak = 0;
    m_currentWinStreak = 0;
    m_lastWinner.clear();

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
