# -------------------------------------------------
# Project created by QtCreator 2009-11-13T00:20:42
# -------------------------------------------------
QT += network \
    xml
QT -= gui
TARGET = waveserver
TEMPLATE = app
SOURCES += main.cpp \
    ../core/model/waveletdeltaoperation.cpp \
    ../core/model/waveletdelta.cpp \
    ../core/model/structureddocument.cpp \
    ../core/model/documentmutation.cpp \
    model/wavelet.cpp \
    model/waveletdocument.cpp \
    model/participant.cpp \
    model/waveusage.cpp \
    model/wave.cpp \
    model/waveurl.cpp \
    ../core/protocol/waveclient-rpc.pb.cc \
    ../core/protocol/common.pb.cc \
    network/serversocket.cpp \
    network/clientconnection.cpp \
    network/xmppcomponent.cpp \
    network/xmppstanza.cpp \
    ../core/network/rpc.cpp \
    ../core/network/converter.cpp \
    persistence/commitlog.cpp \
    app/settings.cpp \
    protocol/commitlog.pb.cc \
    model/jid.cpp \
    network/servercertificate.cpp \
    model/appliedwaveletdelta.cpp \
    model/signedwaveletdelta.cpp \
    model/signature.cpp \
    model/certificatestore.cpp \
    actor/actor.cpp \
    actor/timeout.cpp \
    actor/recvsignal.cpp \
    actor/actorgroup.cpp
HEADERS += ../core/model/waveletdeltaoperation.h \
    ../core/model/waveletdelta.h \
    ../core/model/structureddocument.h \
    ../core/model/documentmutation.h \
    model/wavelet.h \
    model/waveletdocument.h \
    model/participant.h \
    model/waveusage.h \
    model/wave.h \
    model/waveurl.h \
    ../core/protocol/common.pb.h \
    ../core/protocol/waveclient-rpc.pb.h \
    network/serversocket.h \
    network/clientconnection.h \
    network/xmppcomponent.h \
    network/xmppstanza.h \
    ../core/network/rpc.h \
    ../core/network/converter.h \
    persistence/commitlog.h \
    app/settings.h \
    protocol/commitlog.pb.h \
    model/jid.h \
    network/servercertificate.h \
    model/appliedwaveletdelta.h \
    model/signedwaveletdelta.h \
    model/signature.h \
    model/certificatestore.h \
    actor/timeout.h \
    actor/recvxor.h \
    actor/recvxmpp.h \
    actor/recvsignal.h \
    actor/recv.h \
    actor/imessage.h \
    actor/actor.h \
    actor/actorgroup.h \
    actor/waitingcondition.h
unix:LIBS += -lprotobuf \
    -lcrypto
INCLUDEPATH += ./ \
    ../ \
    ../core/ \
    ../core/protocol
