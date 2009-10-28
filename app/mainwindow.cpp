#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversettingsdialog.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/structureddocument.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "model/participant.h"
#include "model/otprocessor.h"
#include "environment.h"
#include "network/networkadapter.h"
#include "view/wavelistview.h"

#include <QtGlobal>

MainWindow::MainWindow(Environment* environment, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), m_environment(environment), m_inboxView(0), m_waveView(0)
{
    ui->setupUi(this);

    connect( ui->actionServerSettings, SIGNAL(triggered()), SLOT(showServerSettings()));
    connect( ui->actionNewWave, SIGNAL(triggered()), SLOT(newWave()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showServerSettings()
{
    ServerSettingsDialog* dlg = new ServerSettingsDialog(m_environment, this);
    dlg->show();
}

void MainWindow::newWave()
{
    // TODO: This is a crude way of creating new wave IDs
    QString rand;
    rand.setNum( qrand() );
    Wave* wave = m_environment->createWave("w+" + rand); //new Wave(m_environment, m_environment->networkAdapter()->serverName(), "w+" + rand);
    Wavelet* wavelet = wave->wavelet();
    wavelet->addParticipant(m_environment->localUser());

    Blip* b = wavelet->blip("b+b1");
    StructuredDocument* bdoc = b->document();
    DocumentMutation m2;
    QHash<QString,QString> map;
    map["author"] = m_environment->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet->processor()->handleSend( m2, b->id() );
    // todo: This should go away
    m2.apply(bdoc);
    bdoc->print_();

    StructuredDocument* doc = wavelet->document();
    DocumentMutation m1;
    m1.insertStart("conversation");
    map.clear();
    map["id"] = "b+b1";
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.insertEnd();
    wavelet->processor()->handleSend( m1, "conversation" );
    m1.apply(doc);
    // todo: This should go away
    wavelet->updateConversation();
    wavelet->print_();

    // Tell the server about the new wave
    // TODO: This is not final and ugly
    m_environment->networkAdapter()->sendAddParticipant(wavelet, m_environment->localUser());

    // Add the new wave to the inbox and show it.
    m_environment->inbox()->addWave(wave);
    m_inboxView->select(wave);
}
