# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
TARGET = QWave
QT += network
TEMPLATE = app
SOURCES += app/main.cpp \
    app/mainwindow.cpp \
    model/wavelet.cpp \
    model/wave.cpp \
    model/participant.cpp \
    model/blip.cpp \
    model/blipthread.cpp \
    model/structureddocument.cpp \
    view/waveletview.cpp \
    view/blipview.cpp \
    view/waveview.cpp \
    model/documentmutation.cpp \
    view/blipgraphicsitem.cpp \
    view/waveletgraphicsitem.cpp \
    view/participantgraphicsitem.cpp \
    view/caret.cpp \
    app/environment.cpp \
    view/graphicstextitem.cpp \
    view/otadapter.cpp \
    network/networkadapter.cpp \
    protocol/waveclient-rpc.pb.cc \
    protocol/common.pb.cc \
    network/rpc.cpp \
    view/wavelistview.cpp \
    model/wavelist.cpp \
    view/wavedigestgraphicsitem.cpp \
    app/serversettingsdialog.cpp \
    model/waveletdelta.cpp \
    model/otprocessor.cpp \
    model/waveletdeltaoperation.cpp \
    model/wavedigest.cpp \
    model/contacts.cpp \
    view/contactsview.cpp \
    view/searchbox.cpp \
    view/participantlistview.cpp \
    view/titlebar.cpp \
    view/inboxview.cpp \
    view/bigbar.cpp \
    view/addparticipantdialog.cpp \
    view/buttongraphicsitem.cpp
HEADERS += app/mainwindow.h \
    model/wavelet.h \
    model/wave.h \
    model/participant.h \
    model/blip.h \
    model/blipthread.h \
    model/structureddocument.h \
    view/waveletview.h \
    view/blipview.h \
    view/waveview.h \
    model/documentmutation.h \
    view/blipgraphicsitem.h \
    view/waveletgraphicsitem.h \
    view/participantgraphicsitem.h \
    view/caret.h \
    app/environment.h \
    view/graphicstextitem.h \
    view/otadapter.h \
    network/networkadapter.h \
    protocol/waveclient-rpc.pb.h \
    protocol/common.pb.h \
    network/rpc.h \
    view/wavelistview.h \
    model/wavelist.h \
    view/wavedigestgraphicsitem.h \
    app/serversettingsdialog.h \
    model/waveletdelta.h \
    model/otprocessor.h \
    model/waveletdeltaoperation.h \
    model/wavedigest.h \
    model/contacts.h \
    view/contactsview.h \
    view/searchbox.h \
    view/participantlistview.h \
    view/titlebar.h \
    view/inboxview.h \
    view/bigbar.h \
    view/addparticipantdialog.h \
    view/buttongraphicsitem.h
FORMS += app/mainwindow.ui \
    app/serversettingsdialog.ui
unix:LIBS += -lprotobuf \
    -lqca
INCLUDEPATH += /usr/include/QtCrypto
