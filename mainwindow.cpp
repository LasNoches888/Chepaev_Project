#include "mainwindow.h"
#include "gamewidget.h"
#include "statsmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QPixmap>
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    stack(new QStackedWidget(this)),
    menuPage(nullptr),
    gamePage(nullptr)
{
    setWindowTitle("Чепаев");
    resize(900, 900);

    createMenuPage();

    setCentralWidget(stack);
    stack->addWidget(menuPage);
    stack->setCurrentWidget(menuPage);
}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (menuPage) {
        QLabel *bg = menuPage->findChild<QLabel*>("background");
        if (bg) {
            bg->update();
        }
    }
}

void MainWindow::createMenuPage()
{
    menuPage = new QWidget(this);

    // Фон
    QLabel *bg = new QLabel(menuPage);
    bg->setObjectName("background");

    QPixmap bgPixmap(":/images/menu_bg.jpg");
    if (!bgPixmap.isNull()) {
        bg->setPixmap(bgPixmap);
        bg->setScaledContents(true);
    } else {
        bg->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2c3e50, stop:1 #34495e);");
    }

    QVBoxLayout *bgLayout = new QVBoxLayout(menuPage);
    bgLayout->addWidget(bg);
    bgLayout->setContentsMargins(0, 0, 0, 0);

    // Контейнер для всего контента
    QWidget *contentContainer = new QWidget(bg);
    contentContainer->setObjectName("contentContainer");
    contentContainer->setStyleSheet(
        "QWidget#contentContainer {"
        "  background: rgba(0, 0, 0, 150);"
        "  border-radius: 15px;"
        "}"
        );

    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    // Заголовок
    QLabel *titleLabel = new QLabel("🎯 ЧЕПАЕВ");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #f39c12; margin-bottom: 20px;");
    contentLayout->addWidget(titleLabel);

    // Статистика (простая надпись) - ДОБАВЛЕНО ОБЪЕКТНОЕ ИМЯ
    StatsManager stats;
    QLabel *statsLabel = new QLabel();
    statsLabel->setObjectName("statsLabel"); // ВАЖНО: добавляем объектное имя
    if (stats.totalGames() > 0) {
        statsLabel->setText(
            QString("📊 Статистика: ⚪ %1%  ⚫ %2%  🤝 %3%")
                .arg(stats.whiteWinPercent(), 0, 'f', 1)
                .arg(stats.blackWinPercent(), 0, 'f', 1)
                .arg(stats.drawPercent(), 0, 'f', 1)
            );
    } else {
        statsLabel->setText("🎯 Сыграйте первую игру!");
    }
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setStyleSheet("font-size: 16px; color: #ecf0f1; font-weight: bold; padding: 10px;");
    contentLayout->addWidget(statsLabel);

    // Кнопки
    btnNewGame = new QPushButton("🎮 Новая игра");
    btnResetStats = new QPushButton("🔄 Сбросить статистику");
    btnExit = new QPushButton("🚪 Выход");

    // Стили кнопок
    QString buttonStyle =
        "QPushButton {"
        "  font-size: 18px;"
        "  background-color: rgba(255,255,255,220);"
        "  border-radius: 10px;"
        "  font-weight: bold;"
        "  min-height: 50px;"
        "  color: black;"
        "  padding: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(255,255,255,180);"
        "}";

    QString resetButtonStyle =
        "QPushButton {"
        "  font-size: 16px;"
        "  background-color: rgba(231, 76, 60, 200);"
        "  border-radius: 10px;"
        "  font-weight: bold;"
        "  min-height: 40px;"
        "  color: white;"
        "  padding: 8px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(231, 76, 60, 150);"
        "}";

    btnNewGame->setStyleSheet(buttonStyle);
    btnResetStats->setStyleSheet(resetButtonStyle);
    btnExit->setStyleSheet(buttonStyle);

    contentLayout->addWidget(btnNewGame);
    contentLayout->addWidget(btnResetStats);
    contentLayout->addWidget(btnExit);

    // Центрируем контейнер
    QHBoxLayout *centerLayout = new QHBoxLayout(bg);
    centerLayout->addStretch();
    centerLayout->addWidget(contentContainer);
    centerLayout->addStretch();

    contentContainer->setMinimumWidth(400);
    contentContainer->setMaximumWidth(500);

    // Подключения
    connect(btnNewGame, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(btnResetStats, &QPushButton::clicked, this, &MainWindow::resetStats);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::exitGame);
}

void MainWindow::startNewGame()
{
    gamePage = new GameWidget(this);
    stack->addWidget(gamePage);
    stack->setCurrentWidget(gamePage);

    connect(gamePage, &GameWidget::gameEnded, this, &MainWindow::handleGameEnd);
    connect(gamePage, &GameWidget::backToMenuClicked, this, &MainWindow::backToMenuFromGame);
}

void MainWindow::resetStats()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Сброс статистики");
    msgBox.setText("Вы уверены, что хотите сбросить всю статистику?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        StatsManager stats;
        stats.reset();

        // ОБНОВЛЕННЫЙ ПОИСК - используем то же объектное имя
        QLabel *statsLabel = menuPage->findChild<QLabel*>("statsLabel");
        if (statsLabel) {
            statsLabel->setText("🎯 Сыграйте первую игру!");
        }
    }
}

void MainWindow::backToMenuFromGame()
{
    if (gamePage) {
        stack->removeWidget(gamePage);
        gamePage->deleteLater();
        gamePage = nullptr;
    }
    stack->setCurrentWidget(menuPage);

    // Обновляем статистику при возврате в меню
    StatsManager stats;
    QLabel *statsLabel = menuPage->findChild<QLabel*>("statsLabel");
    if (statsLabel) {
        if (stats.totalGames() > 0) {
            statsLabel->setText(
                QString("📊 Статистика: ⚪ %1%  ⚫ %2%  🤝 %3%")
                    .arg(stats.whiteWinPercent(), 0, 'f', 1)
                    .arg(stats.blackWinPercent(), 0, 'f', 1)
                    .arg(stats.drawPercent(), 0, 'f', 1)
                );
        } else {
            statsLabel->setText("🎯 Сыграйте первую игру!");
        }
    }
}

void MainWindow::handleGameEnd(const QString &winner)
{
    QString text;
    if (winner == "white") text = "⚪ Белые победили!";
    else if (winner == "black") text = "⚫ Чёрные победили!";
    else text = "🤝 Ничья!";

    StatsManager stats;
    stats.addGameResult(winner);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Игра окончена");
    msgBox.setText(text);
    msgBox.setStyleSheet(
        "QMessageBox {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2c3e50, stop:1 #34495e);"
        "  color: white;"
        "}"
        "QMessageBox QLabel {"
        "  color: white;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        "QMessageBox QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 8px 16px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 80px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        );
    msgBox.exec();

    backToMenuFromGame();
}

void MainWindow::exitGame()
{
    close();
}
