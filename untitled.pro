QT       += core gui widgets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    gamewidget.cpp \
    gamelogic.cpp \
    statsmanager.cpp

HEADERS += \
    mainwindow.h \
    gamewidget.h \
    gamelogic.h \
    statsmanager.h

RESOURCES += \
    menu.qrc
