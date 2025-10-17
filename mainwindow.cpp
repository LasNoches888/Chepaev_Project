#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // создаём виджет игры и вставляем в главное окно
    GameWidget *game = new GameWidget(this);
    setCentralWidget(game);
}

MainWindow::~MainWindow()
{
    delete ui;
}
