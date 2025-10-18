#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QWidget>

class GameWidget;      // вперёд объявляем, чтобы не подключать весь заголовок
class StatsManager;
class SettingsDialog;

/**
 * @brief Главное окно приложения "Чепаев"
 *
 * Содержит главное меню (с кнопками) и экран игры.
 * После окончания партии возвращает игрока в меню.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /// Начать новую игру
    void startNewGame();

    /// Показать статистику
    void showStats();

    /// Показать настройки
    void showSettings();

    /// Выйти из игры
    void exitGame();

private:
    QWidget *menuWidget;     ///< Главное меню
    GameWidget *gameWidget;  ///< Игровой экран

    QPushButton *btnNewGame;
    QPushButton *btnStats;
    QPushButton *btnSettings;
    QPushButton *btnExit;

    void createMenu();   ///< Создаёт меню
    void showMenu();     ///< Показывает меню
    void showGame();     ///< Показывает экран игры
};

#endif // MAINWINDOW_H
