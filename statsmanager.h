#ifndef STATSMANAGER_H
#define STATSMANAGER_H

#include <QObject>
#include <QSettings>

class StatsManager : public QObject
{
    Q_OBJECT
public:
    explicit StatsManager(QObject *parent = nullptr);

    void addGameResult(const QString &winner); // "white", "black", "draw"
    void reset();

    int totalGames() const { return m_totalGames; }
    int whiteWins() const { return m_whiteWins; }
    int blackWins() const { return m_blackWins; }
    int draws() const { return m_draws; }

    double whiteWinPercent() const;
    double blackWinPercent() const;
    double drawPercent() const;

private:
    int m_totalGames;
    int m_whiteWins;
    int m_blackWins;
    int m_draws;

    void load();
    void save() const;
};

#endif // STATSMANAGER_H
