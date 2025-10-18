#include "mainwindow.h"
#include "gamewidget.h"
#include "statsmanager.h"
#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    menuWidget(nullptr),
    gameWidget(nullptr)
{
    setWindowTitle("Чепаев");
    resize(800, 800);

    createMenu();
    showMenu();
}

MainWindow::~MainWindow() {}


// === СОЗДАНИЕ МЕНЮ ===
void MainWindow::createMenu()
{
    menuWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(menuWidget);

    btnNewGame = new QPushButton("Новая игра");
    btnStats = new QPushButton("Статистика");
    btnSettings = new QPushButton("Настройки");
    btnExit = new QPushButton("Выход");

    layout->addStretch();
    layout->addWidget(btnNewGame);
    layout->addWidget(btnStats);
    layout->addWidget(btnSettings);
    layout->addWidget(btnExit);
    layout->addStretch();

    menuWidget->setLayout(layout);

    // подключаем сигналы
    connect(btnNewGame, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::showStats);
    connect(btnSettings, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::exitGame);
}


// === ПОКАЗ МЕНЮ ===
void MainWindow::showMenu()
{
    if (gameWidget) {
        // скрываем и удаляем игру, но не закрываем всё окно
        gameWidget->deleteLater();
        gameWidget = nullptr;
    }
    setCentralWidget(menuWidget);
    menuWidget->show();
}


// === ЗАПУСК ИГРЫ ===
void MainWindow::startNewGame()
{
    showGame();
}


// === ПОКАЗ ИГРОВОГО ПОЛЯ ===
void MainWindow::showGame()
{
    gameWidget = new GameWidget(this);

    connect(gameWidget, &GameWidget::gameEnded, this, [this](const QString &winner){
        QString text;
        if (winner == "white")
            text = "Белые победили!";
        else if (winner == "black")
            text = "Чёрные победили!";
        else
            text = "Ничья!";

        StatsManager stats;
        stats.addGamePlayed();
        if (winner == "white") stats.addWhiteWin();
        else if (winner == "black") stats.addBlackWin();
        else stats.addDraw();

        // Показываем диалог и ждём
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Игра окончена");
        msgBox.setText(text);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.addButton("OK", QMessageBox::AcceptRole);
        msgBox.exec();

        // ⚡ Главное изменение:
        // После закрытия диалога возвращаемся в меню
        showMenu();
    });

    setCentralWidget(gameWidget);
}


// === СТАТИСТИКА ===
void MainWindow::showStats()
{
    StatsManager stats;
    QString text = QString("Всего игр: %1\nПобед белых: %2\nПобед чёрных: %3\nНичьих: %4")
                       .arg(stats.totalGames())
                       .arg(stats.whiteWins())
                       .arg(stats.blackWins())
                       .arg(stats.draws());


    QMessageBox::information(this, "Статистика", text);
}


// === НАСТРОЙКИ ===
void MainWindow::showSettings()
{
    SettingsDialog dialog(this);
    dialog.exec();
}


// === ВЫХОД ===
void MainWindow::exitGame()
{
    close();
}
