# -------------------------------------------------
# Project created by QtCreator 2010-03-01T19:58:04
# -------------------------------------------------
QT -= gui
TARGET = protojs
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    cppjsongenerator.cpp
HEADERS += cppjsongenerator.h
LIBS += -lprotobuf -lprotoc
