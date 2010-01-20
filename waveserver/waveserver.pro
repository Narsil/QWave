# -------------------------------------------------
# Project created by QtCreator 2009-11-13T00:20:42
# -------------------------------------------------
QT += network
TARGET = waveserver
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    ../model/waveletdeltaoperation.cpp \
    ../model/waveletdelta.cpp \
    ../model/structureddocument.cpp \
    ../model/documentmutation.cpp \
    model/wavelet.cpp \
    model/waveletdocument.cpp \
    model/participant.cpp \
    model/waveusage.cpp \
    model/wave.cpp \
    model/waveurl.cpp \
    ../protocol/waveclient-rpc.pb.cc \
    ../protocol/common.pb.cc \
    network/serversocket.cpp \
    network/clientconnection.cpp \
    ../network/rpc.cpp \
    ../network/converter.cpp
HEADERS += mainwindow.h \
    ../model/waveletdeltaoperation.h \
    ../model/waveletdelta.h \
    ../model/structureddocument.h \
    ../model/documentmutation.h \
    model/wavelet.h \
    model/waveletdocument.h \
    model/participant.h \
    model/waveusage.h \
    model/wave.h \
    model/waveurl.h \
    ../protocol/common.pb.h \
    ../protocol/waveclient-rpc.pb.h \
    network/serversocket.h \
    network/clientconnection.h \
    ../network/rpc.h \
    ../network/converter.h
FORMS += mainwindow.ui
unix:LIBS += -lprotobuf
INCLUDEPATH += ./ ../
