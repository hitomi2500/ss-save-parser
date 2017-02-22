#-------------------------------------------------
#
# Project created by QtCreator 2015-02-11T23:42:29
#
#-------------------------------------------------

QT       += core gui

VERSION = 0.9.5
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QMAKE_LFLAGS += -static -static-libgcc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SS_Backup_RAM_Parser
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    setupwindow.cpp \
    config.cpp \
    parselib.cpp \
    entersavedetailsdialog.cpp \
    fileiosettingsdialog.cpp \
    newdialog.cpp \
    imagemapwindow.cpp \
    containerdialog.cpp

HEADERS  += mainwindow.h \
    setupwindow.h \
    config.h \
    parselib.h \
    entersavedetailsdialog.h \
    fileiosettingsdialog.h \
    newdialog.h \
    imagemapwindow.h \
    containerdialog.h

FORMS    += mainwindow.ui \
    setupwindow.ui \
    entersavedetailsdialog.ui \
    fileiosettingsdialog.ui \
    newdialog.ui \
    imagemapwindow.ui \
    containerdialog.ui

DISTFILES +=

RESOURCES += \
    resources.qrc
