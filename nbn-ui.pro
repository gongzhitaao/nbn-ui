#-------------------------------------------------
#
# Project created by QtCreator 2013-12-27T10:25:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = nbn-ui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    parserthread.cpp \
    nbn.cpp \
    nninfomodel.cpp \
    trainthread.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    parserthread.h \
    nbn.h \
    nninfomodel.h \
    trainthread.h

FORMS    += mainwindow.ui

CONFIG += c++11
INCLUDEPATH += /usr/include/eigen3/
