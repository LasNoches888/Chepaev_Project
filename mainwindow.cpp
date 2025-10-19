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

    // Контейнер для кнопок
    QWidget *buttonContainer = new QWidget(bg);
    buttonContainer->setObjectName("buttonContainer");
    buttonContainer->setStyleSheet(
        "QWidget#buttonContainer {"
        "  background: rgba(0, 0, 0, 120);"
        "  border-radius: 15px;"
        "}"
        );

    // Кнопки
    btnNewGame = new QPushButton("Новая игра");
    btnStats = new QPushButton("Статистика");
    btnExit = new QPushButton("Выход");

    // Стили кнопок
    QString buttonStyle =
        "QPushButton {"
        "  font-size: 20px;"
        "  background-color: rgba(255,255,255,220);"
        "  border-radius: 10px;"
        "  font-weight: bold;"
        "  min-height: 50px;"
        "  color: black;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(255,255,255,180);"
        "}";

    btnNewGame->setStyleSheet(buttonStyle);
    btnStats->setStyleSheet(buttonStyle);
    btnExit->setStyleSheet(buttonStyle);

    // Компоновка кнопок
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnNewGame);
    buttonLayout->addWidget(btnStats);
    buttonLayout->addWidget(btnExit);
    buttonLayout->addStretch();
    buttonLayout->setSpacing(15);
    buttonLayout->setContentsMargins(30, 30, 30, 30);

    // Центрируем контейнер
    QHBoxLayout *centerLayout = new QHBoxLayout(bg);
    centerLayout->addStretch();
    centerLayout->addWidget(buttonContainer);
    centerLayout->addStretch();

    buttonContainer->setMinimumWidth(350);
    buttonContainer->setMaximumWidth(500);

    // Подключения
    connect(btnNewGame, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::showStats);
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

void MainWindow::backToMenuFromGame()
{
    if (gamePage) {
        stack->removeWidget(gamePage);
        gamePage->deleteLater();
        gamePage = nullptr;
    }
    stack->setCurrentWidget(menuPage);
}

void MainWindow::handleGameEnd(const QString &winner)
{
    QString text;
    if (winner == "white") text = "⚪ Белые победили!";
    else if (winner == "black") text = "⚫ Чёрные победили!";
    else text = "🤝 Ничья!";

    StatsManager stats;
    stats.addGameResult(winner); // Используем новый метод

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

void MainWindow::showStats()
{
    StatsManager stats;

    QDialog statsDialog(this);
    statsDialog.setWindowTitle("Статистика игр");
    statsDialog.setFixedSize(450, 400);
    statsDialog.setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2c3e50, stop:1 #34495e);"
        "  border-radius: 15px;"
        "  color: white;"
        "}"
        "QLabel {"
        "  color: white;"
        "}"
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(&statsDialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // Заголовок
    QLabel *titleLabel = new QLabel("📊 Статистика игр");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f39c12; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Основная статистика
    QWidget *statsContainer = new QWidget();
    statsContainer->setStyleSheet(
        "background: rgba(255,255,255,15);"
        "border-radius: 10px;"
        "padding: 15px;"
        );

    QGridLayout *statsLayout = new QGridLayout(statsContainer);
    statsLayout->setVerticalSpacing(12);
    statsLayout->setHorizontalSpacing(20);

    // Заголовки столбцов
    QLabel *typeHeader = new QLabel("Тип");
    QLabel *countHeader = new QLabel("Количество");
    QLabel *percentHeader = new QLabel("Процент");

    QString headerStyle = "font-weight: bold; font-size: 14px; color: #bdc3c7;";
    typeHeader->setStyleSheet(headerStyle);
    countHeader->setStyleSheet(headerStyle);
    percentHeader->setStyleSheet(headerStyle);

    statsLayout->addWidget(typeHeader, 0, 0);
    statsLayout->addWidget(countHeader, 0, 1);
    statsLayout->addWidget(percentHeader, 0, 2);

    // Данные статистики
    QStringList types = {"Всего игр", "Победы белых", "Победы черных", "Ничьи"};
    QStringList values = {
        QString::number(stats.totalGames()),
        QString::number(stats.whiteWins()),
        QString::number(stats.blackWins()),
        QString::number(stats.draws())
    };
    QStringList percents = {
        "100%",
        QString("%1%").arg(stats.whiteWinPercent(), 0, 'f', 1),
        QString("%1%").arg(stats.blackWinPercent(), 0, 'f', 1),
        QString("%1%").arg(stats.drawPercent(), 0, 'f', 1)
    };
    QStringList icons = {"🎮", "⚪", "⚫", "🤝"};

    for (int i = 0; i < types.size(); ++i) {
        QLabel *iconLabel = new QLabel(icons[i]);
        QLabel *typeLabel = new QLabel(types[i]);
        QLabel *valueLabel = new QLabel(values[i]);
        QLabel *percentLabel = new QLabel(percents[i]);

        iconLabel->setStyleSheet("font-size: 18px;");
        typeLabel->setStyleSheet("font-size: 14px;");
        valueLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
        percentLabel->setStyleSheet("font-size: 14px; color: #e74c3c; font-weight: bold;");

        statsLayout->addWidget(iconLabel, i + 1, 0);
        statsLayout->addWidget(typeLabel, i + 1, 1);
        statsLayout->addWidget(valueLabel, i + 1, 2);
        statsLayout->addWidget(percentLabel, i + 1, 3);
    }

    mainLayout->addWidget(statsContainer);

    // Графическое представление
    if (stats.totalGames() > 0) {
        QLabel *chartLabel = new QLabel(
            QString("📈 Распределение: ⚪ %1%  ⚫ %2%  🤝 %3%")
                .arg(stats.whiteWinPercent(), 0, 'f', 1)
                .arg(stats.blackWinPercent(), 0, 'f', 1)
                .arg(stats.drawPercent(), 0, 'f', 1)
            );
        chartLabel->setAlignment(Qt::AlignCenter);
        chartLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2ecc71; margin: 10px;");
        mainLayout->addWidget(chartLabel);
    } else {
        QLabel *noDataLabel = new QLabel("🎯 Сыграйте первую игру!");
        noDataLabel->setAlignment(Qt::AlignCenter);
        noDataLabel->setStyleSheet("font-size: 16px; color: #95a5a6; font-style: italic; margin: 20px;");
        mainLayout->addWidget(noDataLabel);
    }

    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *resetButton = new QPushButton("🔄 Сбросить");
    resetButton->setStyleSheet("background-color: #e74c3c;");
    connect(resetButton, &QPushButton::clicked, [&stats, &statsDialog]() {
        stats.reset();
        statsDialog.accept();
    });

    QPushButton *closeButton = new QPushButton("✅ Закрыть");
    connect(closeButton, &QPushButton::clicked, &statsDialog, &QDialog::accept);

    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);

    statsDialog.exec();
}


void MainWindow::exitGame()
{
    close();
}
