QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    gamewidget.cpp \
    checker.cpp \
    player.cpp \
    gamelogic.cpp \
    statsmanager.cpp \
    settingsdialog.cpp

HEADERS += \
    mainwindow.h \
    gamewidget.h \
    checker.h \
    player.h \
    gamelogic.h \
    settingsdialog.h \
    statsmanager.h \
    statsmanager.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui
