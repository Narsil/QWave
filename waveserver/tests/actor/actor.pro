# -------------------------------------------------
# Project created by QtCreator 2010-02-05T17:00:50
# -------------------------------------------------
QT -= gui
TARGET = actor

# CONFIG += console
# CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    ../../actor/actor.cpp \
    ../../actor/timeout.cpp \
    example.cpp \
    ../../actor/actorgroup.cpp \
    ../../actor/recvsignal.cpp \
    ../../network/xmppstanza.cpp
HEADERS += ../../actor/actor.h \
    ../../actor/waitingcondition.h \
    ../../actor/recvxmpp.h \
    ../../network/xmppstanza.h \
    ../../actor/timeout.h \
    example.h \
    ../../actor/imessage.h \
    ../../actor/recv.h \
    ../../actor/actorgroup.h \
    ../../actor/recvxor.h \
    ../../actor/recvsignal.h
INCLUDEPATH += ./ \
    ../../ \
    ../../actor
