#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class GameWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void startNewGame();
    void resetStats();  // Новая функция
    void exitGame();
    void handleGameEnd(const QString &winner);
    void backToMenuFromGame();

private:
    QStackedWidget *stack;
    QWidget *menuPage;
    GameWidget *gamePage;

    QPushButton *btnNewGame;
    QPushButton *btnResetStats;  // Новая кнопка
    QPushButton *btnExit;

    QComboBox *difficultyCombo; // селектор сложности бота
    QLabel *statsLabel;         // отображаемая статистика в меню

    void createMenuPage();
};

#endif // MAINWINDOW_H
