# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
CONFIG += debug
TARGET = randomot
QT += network testlib
TEMPLATE = app
SOURCES += main.cpp \
    ../../model/wavelet.cpp \
    ../../model/wave.cpp \
    ../../model/participant.cpp \
    ../../model/blip.cpp \
    ../../model/blipthread.cpp \
    ../../../core/model/structureddocument.cpp \
    ../../../core/model/documentmutation.cpp \
    ../../app/environment.cpp \
    ../../network/networkadapter.cpp \
    ../../../core/network/rpc.cpp \
    ../../../core/network/converter.cpp \
    ../../model/wavelist.cpp \
    ../../../core/model/waveletdelta.cpp \
    ../../model/otprocessor.cpp \
    ../../../core/model/waveletdeltaoperation.cpp \
    ../../model/wavedigest.cpp \
    ../../model/contacts.cpp \
    ../../app/settings.cpp \
    ../../model/unknowndocument.cpp \
    ../../model/blipdocument.cpp \
    ../../model/attachment.cpp \
    ../../../core/model/waveurl.cpp \
    ../../../core/protocol/waveclient-rpc.pb.cc \
    ../../../core/protocol/common.pb.cc
HEADERS += ../../model/wavelet.h \
    ../../model/wave.h \
    ../../model/participant.h \
    ../../model/blip.h \
    ../../model/blipthread.h \
    ../../../core/model/structureddocument.h \
    ../../../core/model/documentmutation.h \
    ../../app/environment.h \
    ../../network/networkadapter.h \
    ../../../core/network/rpc.h \
    ../../../core/network/converter.h \
    ../../model/wavelist.h \
    ../../../core/model/waveletdelta.h \
    ../../model/otprocessor.h \
    ../../../core/model/waveletdeltaoperation.h \
    ../../model/wavedigest.h \
    ../../model/contacts.h \
    ../../app/settings.h \
    ../../model/unknowndocument.h \
    ../../model/blipdocument.h \
    ../../model/attachment.h \
    ../../../core/model/waveurl.h \
    ../../../core/protocol/waveclient-rpc.pb.h \
    ../../../core/protocol/common.pb.h
unix:LIBS += -lprotobuf
unix:INCLUDEPATH = ../../protocol ../../ ../../../core/
