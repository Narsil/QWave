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
#include "view/inboxview.h"

#include <QtGlobal>
#include <QLabel>

MainWindow::MainWindow(Environment* environment, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), m_environment(environment), m_inboxView(0), m_waveView(0)
{
    ui->setupUi(this);

    m_connectionStatus = new QLabel();
    ui->statusBar->addWidget(m_connectionStatus);

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
    Wave* wave = m_environment->createWave( m_environment->networkAdapter()->serverName(), "w+" + rand); //new Wave(m_environment, m_environment->networkAdapter()->serverName(), "w+" + rand);
    // Add the new wave to the inbox and show it.
    m_environment->inbox()->addWave(wave);
    Wavelet* wavelet = wave->wavelet();

    // Tell the server about the new wave
    // TODO: This is not final and ugly
    wavelet->processor()->handleSendAddParticipant(m_environment->localUser());
    // DEBUG
    // wavelet->processor()->handleSendAddParticipant(new Participant("HeinzWurst@localhost"));
    // m_environment->networkAdapter()->sendAddParticipant(wavelet, m_environment->localUser());

    //StructuredDocument* doc = wavelet->document();
    DocumentMutation m1;
    m1.insertStart("conversation");
    QHash<QString,QString> map;
    map["id"] = "b+b1";
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.insertEnd();
    wavelet->processor()->handleSend( m1, "conversation" );

    DocumentMutation m2;
    map.clear();
    map["name"] = m_environment->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet->processor()->handleSend( m2, "b+b1" );

    m_inboxView->select(wave);
}

void MainWindow::newWave(Participant* p)
{
    // TODO: This is a crude way of creating new wave IDs
    QString rand;
    rand.setNum( qrand() );
    Wave* wave = m_environment->createWave( m_environment->networkAdapter()->serverName(), "w+" + rand);
    // Add the new wave to the inbox and show it.
    m_environment->inbox()->addWave(wave);
    Wavelet* wavelet = wave->wavelet();

    // Tell the server about the new wave
    wavelet->processor()->handleSendAddParticipant(m_environment->localUser());
    wavelet->processor()->handleSendAddParticipant(p);

    DocumentMutation m1;
    m1.insertStart("conversation");
    QHash<QString,QString> map;
    map["id"] = "b+b1";
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.insertEnd();
    wavelet->processor()->handleSend( m1, "conversation" );

    DocumentMutation m2;
    map.clear();
    map["name"] = m_environment->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet->processor()->handleSend( m2, "b+b1" );

    m_inboxView->select(wave);
}

void MainWindow::setConnectionStatus( const QString& status )
{
    m_connectionStatus->setText( status );
}
