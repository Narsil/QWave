# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
CONFIG += debug
TARGET = remoteot
QT += network testlib
TEMPLATE = app
SOURCES += main.cpp \
    ../../model/wavelet.cpp \
    ../../model/wave.cpp \
    ../../model/participant.cpp \
    ../../model/blip.cpp \
    ../../model/blipthread.cpp \
    ../../model/structureddocument.cpp \
    ../../model/documentmutation.cpp \
    ../../app/environment.cpp \
    ../../network/networkadapter.cpp \
    ../../network/rpc.cpp \
    ../../model/wavelist.cpp \
    ../../model/waveletdelta.cpp \
    ../../model/otprocessor.cpp \
    ../../model/waveletdeltaoperation.cpp \
    ../../model/wavedigest.cpp \
    ../../model/contacts.cpp \
    ../../app/settings.cpp \
    ../../model/unknowndocument.cpp \
    ../../model/blipdocument.cpp \
    ../../model/attachment.cpp \
    ../../protocol/waveclient-rpc.pb.cc \
    ../../protocol/common.pb.cc
HEADERS += ../../model/wavelet.h \
    ../../model/wave.h \
    ../../model/participant.h \
    ../../model/blip.h \
    ../../model/blipthread.h \
    ../../model/structureddocument.h \
    ../../model/documentmutation.h \
    ../../app/environment.h \
    ../../network/networkadapter.h \
    ../../network/rpc.h \
    ../../model/wavelist.h \
    ../../model/waveletdelta.h \
    ../../model/otprocessor.h \
    ../../model/waveletdeltaoperation.h \
    ../../model/wavedigest.h \
    ../../model/contacts.h \
    ../../app/settings.h \
    ../../model/unknowndocument.h \
    ../../model/blipdocument.h \
    ../../model/attachment.h \
    ../../protocol/waveclient-rpc.pb.h \
    ../../protocol/common.pb.h
unix:LIBS += -lprotobuf
unix:INCLUDEPATH = ../../protocol ../../
