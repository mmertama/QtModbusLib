#-------------------------------------------------
#
# Project created by QtCreator 2015-07-02T11:26:06
#
#-------------------------------------------------


QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = QtModBusTest
TEMPLATE = app

INCLUDEPATH += ../..

SOURCES += main.cpp\
        mainwindow.cpp \
    miniconnection.cpp \
    calculator.cpp

HEADERS  += mainwindow.h \
    miniconnection.h \
    calculator.h

FORMS    += mainwindow.ui

unix:LLIBF=unix
macx:LLIBF=osx


APP_SEARCH_PATH=$${_PRO_FILE_PWD_}/../../libs/$$LLIBF
message("Note: non system search path:")
message($$APP_SEARCH_PATH)

LIBS += -L$$APP_SEARCH_PATH -lqtmodbus
