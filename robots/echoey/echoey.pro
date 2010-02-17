# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
# CONFIG += release
TARGET = echoey
target.depends = robots
robots.commands = cd ../ && qmake && make
QMAKE_EXTRA_TARGETS += robots
PRE_TARGETDEPS += robots
QT += network \
    xml
# QT -= gui
TEMPLATE = app
SOURCES += main.cpp \
    echoey.cpp
HEADERS += echoey.h
unix:LIBS += -lprotobuf -lqwaverobot -lqwavecore -L../ -L../../core
INCLUDEPATH += ../../core/ \
    ../
