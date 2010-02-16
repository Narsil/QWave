# -------------------------------------------------
# Project created by QtCreator 2009-10-11T22:32:00
# -------------------------------------------------
# CONFIG += release

# Uncomment this line to get the tabbed GUI.
# Tabbed GUI is useful on small mobile devices such as the N900.
# DEFINES += TABGUI

TARGET = QWave
target.depends = core
core.commands = cd ../core && qmake && make
QMAKE_EXTRA_TARGETS += core
PRE_TARGETDEPS += core
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
    view/waveletview.cpp \
    view/blipview.cpp \
    view/waveview.cpp \
    view/blipgraphicsitem.cpp \
    view/waveletgraphicsitem.cpp \
    view/participantgraphicsitem.cpp \
    view/caret.cpp \
    app/environment.cpp \
    view/graphicstextitem.cpp \
    view/otadapter.cpp \
    network/networkadapter.cpp \
    view/wavelistview.cpp \
    model/wavelist.cpp \
    view/wavedigestgraphicsitem.cpp \
    app/serversettingsdialog.cpp \
    model/otprocessor.cpp \
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
    gadgets/gadgetmanifest.cpp \
    gadgets/gadgetapi.cpp \
    gadgets/gadgethandler.cpp \
    gadgets/gadgetview.cpp \
    gadgets/extensionmanifest.cpp \
    view/inboxbuttonview.cpp \
    view/insertgadgetdialog.cpp
HEADERS += app/mainwindow.h \
    model/wavelet.h \
    model/wave.h \
    model/participant.h \
    model/blip.h \
    model/blipthread.h \
    view/waveletview.h \
    view/blipview.h \
    view/waveview.h \
    view/blipgraphicsitem.h \
    view/waveletgraphicsitem.h \
    view/participantgraphicsitem.h \
    view/caret.h \
    app/environment.h \
    view/graphicstextitem.h \
    view/otadapter.h \
    network/networkadapter.h \
    view/wavelistview.h \
    model/wavelist.h \
    view/wavedigestgraphicsitem.h \
    app/serversettingsdialog.h \
    model/otprocessor.h \
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
    gadgets/gadgetmanifest.h \
    gadgets/gadgetapi.h \
    gadgets/gadgethandler.h \
    gadgets/gadgetview.h \
    gadgets/extensionmanifest.h \
    view/inboxbuttonview.h \
    view/insertgadgetdialog.h
FORMS += app/mainwindow.ui \
    app/serversettingsdialog.ui
unix:LIBS += -lprotobuf -lqwavecore -L../core
INCLUDEPATH+= ../core/ ../core/protocol/ 
debug:
release { 
    win32:LIBS += MSVCPRT.LIB \
        msvcrt.lib
}
OTHER_FILES += javascript/gadget.js
