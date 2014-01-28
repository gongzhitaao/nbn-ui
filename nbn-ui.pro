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
    core/nbn.cpp \
    core/util.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    core/nbn.h \
    core/util.h

FORMS    += mainwindow.ui

CONFIG += c++11
INCLUDEPATH += /usr/include/eigen3/
