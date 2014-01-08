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
    nbn.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    nbn.h \
    util.h

FORMS    += mainwindow.ui

CONFIG += c++11
INCLUDEPATH += /usr/include/eigen3/
