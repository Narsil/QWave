# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
# CONFIG += release
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
    network/converter.cpp \
    gadgets/gadgetmanifest.cpp \
    gadgets/gadgetapi.cpp \
    gadgets/gadgethandler.cpp \
    gadgets/gadgetview.cpp \
    gadgets/extensionmanifest.cpp\
    view/inboxbuttonview.cpp
unix:SOURCES += protocol/waveclient-rpc.pb.cc \
    protocol/common.pb.cc
win32:SOURCES += winprotobuf/protocol/waveclient-rpc.pb.cc \
    winprotobuf/protocol/common.pb.cc
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
    network/converter.h \
    gadgets/gadgetmanifest.h \
    gadgets/gadgetapi.h \
    gadgets/gadgethandler.h \
    gadgets/gadgetview.h \
    gadgets/extensionmanifest.h\
    view/inboxbuttonview.h
unix:HEADERS += protocol/waveclient-rpc.pb.h \
    protocol/common.pb.h
win32:HEADERS += winprotobuf/protocol/waveclient-rpc.pb.h \
    winprotobuf/protocol/common.pb.h
FORMS += app/mainwindow.ui \
    app/serversettingsdialog.ui
unix:LIBS += -lprotobuf
win32:INCLUDEPATH = winprotobuf/include \
    winprotobuf/protocol
unix:INCLUDEPATH = protocol
debug:
release { 
    win32:LIBS += MSVCPRT.LIB \
        msvcrt.lib
    win32:LIBS += winprotobuf/lib/release/libprotobuf.lib
}
OTHER_FILES += javascript/gadget.js
