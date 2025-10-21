#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    // Показываем сразу в полноэкранном режиме
    w.showFullScreen();

    return a.exec();
}
