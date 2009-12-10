#include <QtGui/QApplication>
#include <QGraphicsView>
#include <QSplitter>
#include <QDateTime>
#include <QSettings>

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

//#include "protocol/waveclient-rpc.pb.h"
//#include <fstream>
//
//void testprotobuf()
//{
//    std::ofstream str("buf.out");
//    waveserver::ProtocolOpenRequest req;
//    req.set_participant_id("depp@localhost");
//    req.set_wave_id("!indexwave");
//    req.add_wavelet_id_prefix("");
//    req.SerializeToOstream(&str);
//}


int main(int argc, char *argv[])
{
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

        en->localUser()->setPixmap( QPixmap("images/user1.png") );

//        // Configure the server connection
//        en->networkAdapter()->setServer( serverName, port );

        MainWindow* w = new MainWindow(en);
        w->connect( en->networkAdapter(), SIGNAL(connectionStatus(QString)), SLOT(setConnectionStatus(QString)));

        en->configure();

        Participant* p2 = en->contacts()->addParticipant("tux@localhost");
        p2->setName("Tux");
        p2->setPixmap( QPixmap("images/user2.png") );
        Participant* p3 = en->contacts()->addParticipant("kenny@localhost");
        p3->setName("Kenny");
        p3->setPixmap( QPixmap("images/user3.png") );

        Wave* wave = new Wave(en, "localhost", "w+12345");
        // Wavelet* wavelet = new Wavelet(wave, "localhost", "conv+root");
        Wavelet* wavelet = wave->wavelet();
        wavelet->addParticipant(en->localUser());
        wavelet->addParticipant(p2);
        wavelet->addParticipant(p3);

        //    Blip* blip = new Blip(wavelet, "b+b1");
        //    StructuredDocument* doc = blip->document();
        //    doc->beginDelta();
        //    doc->insertChars("Hallo Welt");
        //    doc->endDelta();
        //    BlipThread* thread = new BlipThread(blip, "t+t1");
        //    Blip* blip2 = new Blip(thread, "b+b1");
        //    Blip* blip3 = new Blip(thread, "b+b1");
        StructuredDocument* doc = wavelet->document();
        DocumentMutation m0;
        m0.insertStart("conversation");
        QHash<QString,QString> map;
        map["id"] = "b+b1";
        m0.insertStart("blip", map);
        map.clear();
        map["id"] = "t+t1";
        m0.insertStart("thread", map);
        map.clear();
        map["id"] = "b+b2";
        m0.insertStart("blip", map);
        m0.insertEnd();
        map.clear();
        map["id"] = "b+b3";
        m0.insertStart("blip", map);
        m0.insertEnd();
        m0.insertEnd();
        m0.insertEnd();
        m0.insertEnd();
        bool result = doc->apply(m0, "torben@localhost");
        Q_ASSERT(result);
        doc->print_();
        wavelet->updateConversation("torben@localhost");
        wavelet->print_();

        DocumentMutation m8;
        m8.retain(3);
        map.clear();
        map["id"] = "b+b4";
        m8.insertStart("blip", map);
        m8.insertEnd();
        m8.retain(7);
        result = doc->apply(m8, "torben@localhost");
        Q_ASSERT(result);
        doc->print_();
        wavelet->updateConversation("torben@localhost");
        wavelet->print_();

        DocumentMutation m9;
        m9.retain(5);
        m9.deleteStart("blip", StructuredDocument::AttributeList());
        m9.deleteEnd();
        m9.retain(5);
        result = doc->apply(m9, "torben@localhost");
        Q_ASSERT(result);
        doc->print_();
        wavelet->updateConversation("torben@localhost");
        wavelet->print_();

        Blip* b = wavelet->blip("b+b1");
        StructuredDocument* bdoc = b->document();
        DocumentMutation m1;
        // bdoc->beginDelta();
        map.clear();
        map["name"] = "torben@localhost";
        m1.insertStart("contributor", map);
        m1.insertEnd();
        map.clear();
        m1.insertStart("body", map);
        m1.insertStart("line", map);
        m1.insertEnd();
        m1.insertChars("Hello World ");
        StructuredDocument::AnnotationChange format;
        format["style/fontWeight"].second = "bold";
        m1.annotationBoundary(QList<QString>(), format);
        m1.insertChars("this is bold");
        QList<QString> end;
        end.append("style/fontWeight");
        m1.annotationBoundary(end, StructuredDocument::AnnotationChange());
        m1.insertChars(" and normal again");
        map.clear();
        m1.insertStart("line", map);
        m1.insertEnd();
        m1.insertChars("Here comes a new line");
        m1.insertEnd();
        // bdoc->endDelta();
        b->receive(m1, en->localUser()->address());
        bdoc->print_();

        // bdoc->beginDelta();
        DocumentMutation m2;
        m2.retain(6);
        m2.insertChars("a");
        m2.retain(5);
        StructuredDocument::AnnotationChange format2;
        format2["style/fontStyle"].second = "italic";
        m2.annotationBoundary(QList<QString>(), format2);
        m2.retain(10);
        QList<QString> end2;
        end2.append("style/fontStyle");
        m2.annotationBoundary(end2, StructuredDocument::AnnotationChange());
        m2.retain(49);
        b->receive(m2, en->localUser()->address());
        bdoc->print_();

        b = wavelet->blip("b+b3");
        DocumentMutation m3;
        bdoc = b->document();
        map.clear();
        map["name"] = "tux@localhost";
        m3.insertStart("contributor", map);
        m3.insertEnd();
        map.clear();
        m3.insertStart("body", map);
        m3.insertStart("line", map);
        m3.insertEnd();
        m3.insertChars("Oh no, they killed Kenny");
        m3.insertEnd();
        b->receive(m3, "tux@localhost");
        bdoc->print_();

        b = wavelet->blip("b+b4");
        DocumentMutation m4;
        bdoc = b->document();
        map.clear();
        map["name"] = "kenny@localhost";
        m4.insertStart("contributor", map);
        m4.insertEnd();
        map.clear();
        m4.insertStart("body", map);
        m4.insertStart("line", map);
        m4.insertEnd();
        m4.insertChars("Not dead yet, ... oh wait ... arrrg");
        m4.insertEnd();
        b->receive(m4, "kenny@localhost");
        bdoc->print_();

        /*
    StructuredDocument* mdoc = b->document();
    mdoc->beginDelta();
    mdoc->insertChars("Hello");
    mdoc->endDelta();
    mdoc->print_();
    DocumentMutation mut1;
    mut1.retain(1);
    mut1.deleteChars("e");
    mut1.insertChars("a");
    mut1.retain(3);
    DocumentMutation mut2;
    mut2.retain(5);
    mut2.insertChars(" World");
    DocumentMutation mut3( mut1.translate(mut2) );
    mut3.print_();
    mut1.apply(mdoc);
    mdoc->print_();
    mut3.apply(mdoc);
    mdoc->print_();

    StructuredDocument* cdoc = b->document();
    cdoc->beginDelta();
    cdoc->insertChars("Hello");
    cdoc->endDelta();
    cdoc->print_();
    DocumentMutation cmut1;
    cmut1.retain(1);
    cmut1.deleteChars("ello");
    cmut1.insertChars("i");
    DocumentMutation cmut2;
    cmut2.retain(2);
    cmut2.insertChars(" World");
    DocumentMutation cmut3( cmut1.concat(cmut2) );    
    cmut3.print_();
    cmut3.apply(mdoc);
    mdoc->print_(); */

        QSplitter* splitter = new QSplitter();

        WaveView* view = new WaveView(wave);
        // WaveListView* wlview = new WaveListView(en->inbox());
        InboxView* wlview = new InboxView(en);
        ContactsView* cview = new ContactsView( en->contacts() );

        splitter->addWidget(cview);
        splitter->addWidget(wlview);
        splitter->addWidget(view);
        w->setCentralWidget(splitter);
        w->setInboxView(wlview);
        w->setWaveView(view);
        view->connect( wlview, SIGNAL(selected(Wave*)), SLOT(setWave(Wave*)));
	w->connect( wlview, SIGNAL(newWave()), SLOT(newWave()));
        w->connect( cview, SIGNAL(newWave(Participant*)), SLOT(newWave(Participant*)));
        w->connect( view, SIGNAL(newWave(Participant*)), SLOT(newWave(Participant*)));
        w->show();    
        en->inbox()->addWave(wave);
    // End Test

    //WaveletView* v = new WaveletView(view, wavelet);

//    testprotobuf();

    //    return 0;
    return a.exec();
}

