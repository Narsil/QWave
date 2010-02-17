TEMPLATE = lib
TARGET = qwaverobot
QT += network
CONFIG += staticlib
HEADERS += network/networkadapter.h \
    model/attachment.h \
    model/blip.h \
    model/blipdocument.h \
    model/blipthread.h \
    model/contacts.h \
    model/otprocessor.h \
    model/participant.h \
    model/unknowndocument.h \
    model/wave.h \
    model/wavelet.h \
    model/wavelist.h \
    app/environment.h \
    app/settings.h \
    robot.h
SOURCES += network/networkadapter.cpp \
    model/attachment.cpp \
    model/blip.cpp \
    model/blipdocument.cpp \
    model/blipthread.cpp \
    model/contacts.cpp \
    model/otprocessor.cpp \
    model/participant.cpp \
    model/unknowndocument.cpp \
    model/wave.cpp \
    model/wavelet.cpp \
    model/wavelist.cpp \
    app/environment.cpp \
    app/settings.cpp \
    robot.cpp
INCLUDEPATH += ../core
