#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <QObject>
#include <QSettings>

/**
 * @brief Класс для сохранения и загрузки статистики игр.
 *
 * Сохраняет количество сыгранных игр, побед белых, побед чёрных и ничьих.
 * Использует QSettings, чтобы статистика сохранялась между запусками.
 */
class StatsManager : public QObject
{
    Q_OBJECT
public:
    explicit StatsManager(QObject *parent = nullptr);

    void load();
    void save() const;

    void addGamePlayed();
    void addWhiteWin();
    void addBlackWin();
    void addDraw();

    // 🔹 Публичные методы доступа:
    int totalGames() const { return m_totalGames; }
    int whiteWins()  const { return m_whiteWins; }
    int blackWins()  const { return m_blackWins; }
    int draws()      const { return m_draws; }

private:
    int m_totalGames;
    int m_whiteWins;
    int m_blackWins;
    int m_draws;
};

#endif // STATSMANAGER_H
