#-------------------------------------------------
#
# Project created by QtCreator 2016-05-20T10:25:36
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GPGPUSample
TEMPLATE = app


SOURCES += main.cpp\
        GPGPUSample.cpp \
    QxGeneralCompute.cpp

HEADERS  += GPGPUSample.hpp \
    QxGeneralCompute.hpp

FORMS    += GPGPUSample.ui

LIBS += -lglew32
