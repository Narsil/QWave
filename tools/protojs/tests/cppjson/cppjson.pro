#-------------------------------------------------
#
# Project created by QtCreator 2010-03-03T23:46:56
#
#-------------------------------------------------

QT       -= gui

TARGET = cppjson
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
LIBS += -L../../../libprotojs -lprotojs -lprotobuf

SOURCES += main.cpp \
    common.pbjson.cpp \
    common.pb.cc

HEADERS += common.pbjson.h \
    common.pb.h

INCLUDEPATH += ../../../libprotojs
