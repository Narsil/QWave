TEMPLATE = lib
TARGET = qwavecore
QT += network
QT -= gui
CONFIG += staticlib 
HEADERS += network/converter.h \
    network/rpc.h \
    protocol/common.pb.h \
    protocol/waveclient-rpc.pb.h \
    model/documentmutation.h \
    model/structureddocument.h \
    model/waveletdelta.h \
    model/waveletdeltaoperation.h \
    model/waveurl.h
SOURCES += network/converter.cpp \
    network/rpc.cpp \
    protocol/common.pb.cc \
    protocol/waveclient-rpc.pb.cc \
    model/documentmutation.cpp \
    model/structureddocument.cpp \
    model/waveletdelta.cpp \
    model/waveletdeltaoperation.cpp \
    model/waveurl.cpp
