QT += core
QT -= gui

TARGET = virtualFileSys
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    filesystem.cpp

HEADERS += \
    filesystem.h


