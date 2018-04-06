#-------------------------------------------------
#
# Project created by QtCreator 2018-02-26T20:04:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lab
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    copydialog.cpp \
    refreshdialog.cpp \
    resourcemanager.cpp \
    moduledialog.cpp

HEADERS  += mainwindow.h \
    copydialog.h \
    refreshdialog.h \
    resourcemanager.h \
    moduledialog.h

FORMS    += mainwindow.ui \
    copydialog.ui \
    refreshdialog.ui \
    resourcemanager.ui \
    moduledialog.ui
