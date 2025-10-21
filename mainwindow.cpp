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
#include <QComboBox>
#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    stack(new QStackedWidget(this)),
    menuPage(nullptr),
    gamePage(nullptr),
    btnNewGame(nullptr),
    btnResetStats(nullptr),
    btnExit(nullptr),
    difficultyCombo(nullptr),
    statsLabel(nullptr)
{
    setWindowTitle(QString::fromUtf8("Чепаев"));
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

static QString formatStatsText(const StatsManager &stats)
{
    if (stats.totalGames() == 0) {
        // красивое приглашение начать первую игру
        return QString::fromUtf8(
            "<div style='text-align:center;'>"
            "<h2 style='color:#f1c40f; margin:6px;'>Начните свою первую игру</h2>"
            "<p style='color:#ecf0f1; margin:0;'>Нажмите \"Новая игра\", чтобы сыграть — статистика появится после первой партии.</p>"
            "</div>"
            );
    } else {
        return QString::fromUtf8(
                   "<div style='color:white;'>"
                   "<b>Статистика:</b><br>"
                   "Всего игр: %1<br>"
                   "Белые победы: %2 (%5%)<br>"
                   "Чёрные победы: %3 (%6%)<br>"
                   "Ничьи: %4 (%7%)<br>"
                   "Текущая серия: %8<br>"
                   "Макс. серия побед: %9<br>"
                   "Последний победитель: %10"
                   "</div>"
                   ).arg(stats.totalGames())
            .arg(stats.whiteWins())
            .arg(stats.blackWins())
            .arg(stats.draws())
            .arg(QString::number(stats.whiteWinPercent(), 'f', 1))
            .arg(QString::number(stats.blackWinPercent(), 'f', 1))
            .arg(QString::number(stats.drawPercent(), 'f', 1))
            .arg(stats.currentWinStreak())
            .arg(stats.longestWinStreak())
            .arg(stats.lastWinner().isEmpty() ? QString::fromUtf8("—") : stats.lastWinner());
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
        "  background: rgba(0, 0, 0, 160);"
        "  border-radius: 16px;"
        "}"
        );

    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(14);
    contentLayout->setContentsMargins(28, 28, 28, 28);

    // Заголовок
    QLabel *titleLabel = new QLabel(QString::fromUtf8("\U0001F3AF ЧЕПАЕВ"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #f39c12; margin-bottom: 6px;");
    contentLayout->addWidget(titleLabel);

    // Статистика (используем QLabel с поддержкой HTML)
    statsLabel = new QLabel(contentContainer);
    statsLabel->setObjectName("statsLabel");
    statsLabel->setStyleSheet("font-size: 14px;");
    statsLabel->setWordWrap(true);
    statsLabel->setTextFormat(Qt::RichText);

    // Загрузка данных статистики и установка текста
    {
        StatsManager stats;
        statsLabel->setText(formatStatsText(stats));
    }
    contentLayout->addWidget(statsLabel);

    // Селектор сложности
    difficultyCombo = new QComboBox(contentContainer);
    difficultyCombo->setStyleSheet(
        "QComboBox { background: white; padding: 8px; border-radius: 6px; }"
        "QComboBox QAbstractItemView { selection-background-color: #f39c12; }"
        );
    difficultyCombo->addItem(QString::fromUtf8("Легко"));
    difficultyCombo->addItem(QString::fromUtf8("Средне"));
    difficultyCombo->addItem(QString::fromUtf8("Сложно"));
    difficultyCombo->setCurrentIndex(1); // по умолчанию Medium
    contentLayout->addWidget(difficultyCombo);

    // Большие стильные кнопки
    auto makeButton = [&](const QString &text)->QPushButton* {
        QPushButton *b = new QPushButton(text, contentContainer);
        b->setMinimumHeight(48);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton {"
            "  color: white;"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f39c12, stop:1 #e67e22);"
            "  border: none;"
            "  border-radius: 10px;"
            "  font-size: 16px;"
            "  padding: 8px 16px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffb85a, stop:1 #f39c12);"
            "}"
            "QPushButton:pressed {"
            "  background: #d35400;"
            "}"
            );
        return b;
    };

    btnNewGame = makeButton(QString::fromUtf8("Новая игра"));
    btnResetStats = makeButton(QString::fromUtf8("Сбросить статистику"));
    btnExit = makeButton(QString::fromUtf8("Выход"));

    contentLayout->addWidget(btnNewGame);
    contentLayout->addWidget(btnResetStats);
    contentLayout->addWidget(btnExit);

    // Центрируем контейнер
    QHBoxLayout *centerLayout = new QHBoxLayout(bg);
    centerLayout->addStretch();
    centerLayout->addWidget(contentContainer);
    centerLayout->addStretch();

    contentContainer->setMinimumWidth(420);
    contentContainer->setMaximumWidth(540);

    // Подключения
    connect(btnNewGame, &QPushButton::clicked, this, &MainWindow::startNewGame);
    connect(btnResetStats, &QPushButton::clicked, this, &MainWindow::resetStats);
    connect(btnExit, &QPushButton::clicked, this, &MainWindow::exitGame);
}

void MainWindow::startNewGame()
{
    // Создаём виджет игры
    gamePage = new GameWidget(this);

    // Применяем выбранную сложность
    if (difficultyCombo) {
        int idx = difficultyCombo->currentIndex();
        switch (idx) {
        case 0: gamePage->setBotDifficulty(GameWidget::Easy); break;
        case 1: gamePage->setBotDifficulty(GameWidget::Medium); break;
        case 2: gamePage->setBotDifficulty(GameWidget::Hard); break;
        default: gamePage->setBotDifficulty(GameWidget::Medium); break;
        }
    }

    stack->addWidget(gamePage);
    stack->setCurrentWidget(gamePage);

    connect(gamePage, &GameWidget::gameEnded, this, &MainWindow::handleGameEnd);
    connect(gamePage, &GameWidget::backToMenuClicked, this, &MainWindow::backToMenuFromGame);
}

void MainWindow::resetStats()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QString::fromUtf8("Сброс статистики"));
    msgBox.setText(QString::fromUtf8("Вы уверены, что хотите сбросить всю статистику?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        StatsManager stats;
        stats.reset();

        if (statsLabel) statsLabel->setText(formatStatsText(stats));
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
    if (statsLabel) {
        statsLabel->setText(formatStatsText(stats));
        // Если сыграна хотя бы одна игра, после первой покажем призыв "Сыграйте ещё!" рядом со статистикой
        if (stats.totalGames() > 0) {
            // можно дополнительно добавить визуальное напоминание
        }
    }
}

void MainWindow::handleGameEnd(const QString &winner)
{
    QString text;
    if (winner == "white") text = QString::fromUtf8("\u26AA Белые победили!");
    else if (winner == "black") text = QString::fromUtf8("\u26AB Чёрные победили!");
    else text = QString::fromUtf8("\U0001F91D Ничья!");

    StatsManager stats;
    stats.addGameResult(winner);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QString::fromUtf8("Игра окончена"));
    msgBox.setText(text);
    msgBox.exec();

    // возвращаем в меню и обновляем статистику (текст теперь покажет, что уже есть игры)
    backToMenuFromGame();
}

void MainWindow::exitGame()
{
    close();
}
