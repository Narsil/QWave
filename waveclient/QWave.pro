# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
# CONFIG += release

# Uncomment this line to get the tabbed GUI.
# Tabbed GUI is useful on small mobile devices such as the N900.
# DEFINES += TABGUI

TARGET = QWave
QT += network \
    webkit \
    xml
TEMPLATE = app
SOURCES += app/main.cpp \
    app/mainwindow.cpp \
    model/wavelet.cpp \
    model/wave.cpp \
    model/participant.cpp \
    model/blip.cpp \
    model/blipthread.cpp \
    ../core/model/structureddocument.cpp \
    view/waveletview.cpp \
    view/blipview.cpp \
    view/waveview.cpp \
    ../core/model/documentmutation.cpp \
    view/blipgraphicsitem.cpp \
    view/waveletgraphicsitem.cpp \
    view/participantgraphicsitem.cpp \
    view/caret.cpp \
    app/environment.cpp \
    view/graphicstextitem.cpp \
    view/otadapter.cpp \
    network/networkadapter.cpp \
    ../core/network/rpc.cpp \
    view/wavelistview.cpp \
    model/wavelist.cpp \
    view/wavedigestgraphicsitem.cpp \
    app/serversettingsdialog.cpp \
    ../core/model/waveletdelta.cpp \
    model/otprocessor.cpp \
    ../core/model/waveletdeltaoperation.cpp \
    model/wavedigest.cpp \
    model/contacts.cpp \
    view/contactsview.cpp \
    view/searchbox.cpp \
    view/participantlistview.cpp \
    view/titlebar.cpp \
    view/inboxview.cpp \
    view/bigbar.cpp \
    view/addparticipantdialog.cpp \
    view/buttongraphicsitem.cpp \
    app/settings.cpp \
    model/unknowndocument.cpp \
    model/blipdocument.cpp \
    view/popupdialog.cpp \
    view/participantinfodialog.cpp \
    view/insertimagedialog.cpp \
    view/imagehandler.cpp \
    model/attachment.cpp \
    view/toolbar.cpp \
    ../core/network/converter.cpp \
    gadgets/gadgetmanifest.cpp \
    gadgets/gadgetapi.cpp \
    gadgets/gadgethandler.cpp \
    gadgets/gadgetview.cpp \
    gadgets/extensionmanifest.cpp \
    view/inboxbuttonview.cpp \
    ../core/model/waveurl.cpp \
    view/insertgadgetdialog.cpp
unix:SOURCES += ../core/protocol/waveclient-rpc.pb.cc \
    ../core/protocol/common.pb.cc
win32:SOURCES += ../core/winprotobuf/protocol/waveclient-rpc.pb.cc \
    ../core/winprotobuf/protocol/common.pb.cc
HEADERS += app/mainwindow.h \
    model/wavelet.h \
    model/wave.h \
    model/participant.h \
    model/blip.h \
    model/blipthread.h \
    ../core/model/structureddocument.h \
    view/waveletview.h \
    view/blipview.h \
    view/waveview.h \
    ../core/model/documentmutation.h \
    view/blipgraphicsitem.h \
    view/waveletgraphicsitem.h \
    view/participantgraphicsitem.h \
    view/caret.h \
    app/environment.h \
    view/graphicstextitem.h \
    view/otadapter.h \
    network/networkadapter.h \
    ../core/network/rpc.h \
    view/wavelistview.h \
    model/wavelist.h \
    view/wavedigestgraphicsitem.h \
    app/serversettingsdialog.h \
    ../core/model/waveletdelta.h \
    model/otprocessor.h \
    ../core/model/waveletdeltaoperation.h \
    model/wavedigest.h \
    model/contacts.h \
    view/contactsview.h \
    view/searchbox.h \
    view/participantlistview.h \
    view/titlebar.h \
    view/inboxview.h \
    view/bigbar.h \
    view/addparticipantdialog.h \
    view/buttongraphicsitem.h \
    app/settings.h \
    model/unknowndocument.h \
    model/blipdocument.h \
    view/popupdialog.h \
    view/participantinfodialog.h \
    view/insertimagedialog.h \
    view/imagehandler.h \
    model/attachment.h \
    view/toolbar.h \
    ../core/network/converter.h \
    gadgets/gadgetmanifest.h \
    gadgets/gadgetapi.h \
    gadgets/gadgethandler.h \
    gadgets/gadgetview.h \
    gadgets/extensionmanifest.h \
    view/inboxbuttonview.h \
    ../core/model/waveurl.h \
    view/insertgadgetdialog.h 
unix:HEADERS += ../core/protocol/waveclient-rpc.pb.h \
    ../core/protocol/common.pb.h
win32:HEADERS += ../core/winprotobuf/protocol/waveclient-rpc.pb.h \
    ../core/winprotobuf/protocol/common.pb.h
FORMS += app/mainwindow.ui \
    app/serversettingsdialog.ui
unix:LIBS += -lprotobuf
win32:INCLUDEPATH = ../core/winprotobuf/include \
    ../core/winprotobuf/protocol
unix:INCLUDEPATH = protocol
INCLUDEPATH+= ../core/ ../core/protocol
debug:
release { 
    win32:LIBS += MSVCPRT.LIB \
        msvcrt.lib
    win32:LIBS += winprotobuf/lib/release/libprotobuf.lib
}
OTHER_FILES += javascript/gadget.js
