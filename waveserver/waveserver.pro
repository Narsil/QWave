# -------------------------------------------------
# Project created by QtCreator 2009-11-13T00:20:42
# -------------------------------------------------
QT += network \
    xml
QT -= gui
TARGET = waveserver
target.depends = core
core.commands = cd \
    ../core \
    && \
    qmake \
    && \
    make
QMAKE_EXTRA_TARGETS += core
PRE_TARGETDEPS += core
TEMPLATE = app
SOURCES += main.cpp \
    model/wavelet.cpp \
    model/waveletdocument.cpp \
    model/wave.cpp \
    model/waveurl.cpp \
    network/serversocket.cpp \
    network/clientconnection.cpp \
    network/xmppstanza.cpp \
    persistence/commitlog.cpp \
    app/settings.cpp \
    protocol/messages.pb.cc \
    model/jid.cpp \
    network/servercertificate.cpp \
    model/appliedwaveletdelta.cpp \
    model/signedwaveletdelta.cpp \
    model/signature.cpp \
    model/certificatestore.cpp \
    actor/actor.cpp \
    actor/timeout.cpp \
    actor/recvsignal.cpp \
    actor/actorgroup.cpp \
    network/xmppcomponentconnection.cpp \
    network/xmppvirtualconnection.cpp \
    network/xmppdiscoactor.cpp \
    network/xmppactor.cpp \
    network/xmppwaveletupdateactor.cpp \
    network/xmppdiscoresponseactor.cpp \
    network/xmppsignerresponseactor.cpp \
    network/xmpphistoryresponseactor.cpp \
    network/xmppsubmitresponseactor.cpp \
    network/xmpppostsignerresponseactor.cpp \
    network/xmppwaveletupdateresponseactor.cpp \
    network/xmppsubmitrequestactor.cpp \
    model/remotewavelet.cpp \
    model/localwavelet.cpp \
    network/clientsubmitrequestactor.cpp \
    network/clientactor.cpp \
    actor/actorid.cpp \
    actor/actorfolk.cpp \
    actor/actordispatcher.cpp \
    network/clientactorfolk.cpp \
    model/wavefolk.cpp \
    persistence/storefolk.cpp \
    persistence/store.cpp \
    actor/recvcriticalsection.cpp \
    network/clientindexwaveactor.cpp \
    network/clientparticipant.cpp \
    fcgi/fcgirequest.cpp \
    fcgi/fcgiprotocol.cpp \
    fcgi/fcgiserver.cpp \
    ../core/protocol/common.pbjson.cpp \
    ../core/protocol/waveclient-rpc.pbjson.cpp \
    fcgi/fcgiclientconnection.cpp \
    protocol/webclient.pb.cc \
    protocol/webclient.pbjson.cpp
HEADERS += model/wavelet.h \
    persistence/storefolk.h \
    persistence/store.h \
    actor/recvcriticalsection.h \
    network/clientindexwaveactor.h \
    network/clientparticipant.h \
    model/waveletdocument.h \
    model/wave.h \
    model/waveurl.h \
    network/serversocket.h \
    network/clientconnection.h \
    network/xmppstanza.h \
    persistence/commitlog.h \
    app/settings.h \
    protocol/messages.pb.h \
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
    actor/waitingcondition.h \
    network/xmppcomponentconnection.h \
    network/xmppvirtualconnection.h \
    network/xmppdiscoactor.h \
    network/xmppactor.h \
    network/xmppwaveletupdateactor.h \
    network/xmppdiscoresponseactor.h \
    network/xmppsignerresponseactor.h \
    network/xmpphistoryresponseactor.h \
    network/xmppsubmitresponseactor.h \
    network/xmpppostsignerresponseactor.h \
    network/xmppwaveletupdateresponseactor.h \
    network/xmppsubmitrequestactor.h \
    model/remotewavelet.h \
    model/localwavelet.h \
    network/clientactor.h \
    network/clientsubmitrequestactor.h \
    actor/actorid.h \
    actor/actorfolk.h \
    actor/actordispatcher.h \
    network/clientactorfolk.h \
    actor/pbmessage.h \
    actor/recvpb.h \
    model/wavefolk.h \
    fcgi/fcgirequest.h \
    fcgi/fcgiprotocol.h \
    fcgi/fcgi.h \
    fcgi/fcgiserver.h \
    ../core/protocol/common.pbjson.h \
    ../core/protocol/waveclient-rpc.pbjson.h \
    fcgi/fcgiclientconnection.h \
    protocol/webclient.pb.h \
    protocol/webclient.pbjson.h
unix:LIBS += -lprotobuf \
    -lcrypto \
    -lqwavecore \
    -lprotojs \
    -L../core \
    -L \
    ../tools/libprotojs
INCLUDEPATH += ./ \
    ../ \
    ../core/ \
    ../core/protocol \
    ../tools/libprotojs
