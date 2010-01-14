#include <QtGui/QApplication>
#include <QGraphicsView>
#include <QSplitter>
#include <QDateTime>
#include <QSettings>
#include <QTabWidget>

#include "mainwindow.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blipthread.h"
#include "model/blip.h"
#include "model/contacts.h"
#include "view/waveview.h"
#include "view/waveletview.h"
#include "view/wavelistview.h"
#include "model/documentmutation.h"
#include "model/participant.h"
#include "model/wavelist.h"
#include "app/environment.h"
#include "app/settings.h"
#include "network/networkadapter.h"
#include "model/blipdocument.h"
#include "view/contactsview.h"
#include "view/inboxview.h"
#include "serversettingsdialog.h"

int main(int argc, char *argv[])
{
    // Initialize the random number generator
    qsrand( QDateTime::currentDateTime().toTime_t());

    QApplication a(argc, argv);

    QString profile = "QWaveClient";
    if ( argc == 2 )
        profile = QString(argv[1]);
    Environment* en = new Environment(profile);
    // Configure the local user
    Settings* settings = en->settings();
    if( !settings->isConfigured() )
    {
        ServerSettingsDialog dlg(en);
        if ( dlg.exec() == QDialog::Rejected )
            return 0;
    }

    // Set some hard coded image ... TODO
    en->localUser()->setPixmap( QPixmap("images/user1.png") );

    MainWindow* w = new MainWindow(en);
    w->connect( en->networkAdapter(), SIGNAL(connectionStatus(QString)), SLOT(setConnectionStatus(QString)));

    // Configure the environment according to the settings.
    // This will attempt a server connection
    en->configure();

    WaveView* view = new WaveView();
    InboxView* wlview = new InboxView(en);
    ContactsView* cview = new ContactsView( en->contacts() );

#ifdef TABGUI
        QTabWidget* tabwidget = new QTabWidget();
        tabwidget->addTab(cview, w->tr("Contacts") );
        tabwidget->addTab(wlview, w->tr("Inbox") );
        tabwidget->addTab(view, w->tr("Wave") );
        w->setCentralWidget(tabwidget);
#else
        QSplitter* splitter = new QSplitter();
        splitter->addWidget(cview);
        splitter->addWidget(wlview);
        splitter->addWidget(view);
        w->setCentralWidget(splitter);
#endif

    w->setInboxView(wlview);
    w->setWaveView(view);
    view->connect( wlview, SIGNAL(selected(Wave*)), SLOT(setWave(Wave*)));
    w->connect( wlview, SIGNAL(newWave()), SLOT(newWave()));
    w->connect( cview, SIGNAL(newWave(Participant*)), SLOT(newWave(Participant*)));
    w->connect( view, SIGNAL(newWave(Participant*)), SLOT(newWave(Participant*)));
    w->show();

    return a.exec();
}

