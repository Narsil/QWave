#include <QtGui/QApplication>
#include <QGraphicsView>
#include <QSplitter>
#include <QDateTime>

#include "mainwindow.h"

#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blipthread.h"
#include "model/blip.h"
#include "model/synchronizeddocument.h"
#include "view/waveview.h"
#include "view/waveletview.h"
#include "view/wavelistview.h"
#include "model/documentmutation.h"
#include "model/participant.h"
#include "model/wavelist.h"
#include "app/environment.h"
#include "network/networkadapter.h"

#include "protocol/waveclient-rpc.pb.h"
#include <fstream>

void testprotobuf()
{
    std::ofstream str("buf.out");
    waveserver::ProtocolOpenRequest req;
    req.set_participant_id("depp@localhost");
    req.set_wave_id("!indexwave");
    req.add_wavelet_id_prefix("");
    req.SerializeToOstream(&str);
}


int main(int argc, char *argv[])
{
    qsrand( QDateTime::currentDateTime().toTime_t());

    QApplication a(argc, argv);

    for( int i = 0; i < 1; ++i )
    {
        // Test
        Environment* en = new Environment("torben@localhost", "Torben");
        en->localUser()->setPixmap( QPixmap("images/user1.jpg") );
        en->networkAdapter()->setServer( "localhost", 9876 );

        MainWindow* w = new MainWindow(en);

        Participant* p2 = new Participant("joe@acme.com");
        p2->setName("Tux");
        p2->setPixmap( QPixmap("images/user2.jpg") );
        Participant* p3 = new Participant("pam@foobar.com");
        p3->setName("Kenny");
        p3->setPixmap( QPixmap("images/user3.jpg") );

        Wave* wave = new Wave(en, "localhost", "w+12345");
        wave->setDigest("This is some wave which as a digest which can be longer than the space available in here.");
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
        doc->beginDelta();
        doc->insertStart("conversation");
        QHash<QString,QString> map;
        map["id"] = "b+b1";
        doc->insertStart("blip", map);
        map.clear();
        map["id"] = "t+t1";
        doc->insertStart("thread", map);
        map.clear();
        map["id"] = "b+b2";
        doc->insertStart("blip", map);
        doc->insertEnd();
        map.clear();
        map["id"] = "b+b3";
        doc->insertStart("blip", map);
        doc->insertEnd();
        doc->insertEnd();
        doc->insertEnd();
        doc->insertEnd();
        doc->endDelta();
        wavelet->updateConversation();
        wavelet->print_();

        doc->beginDelta();
        doc->retain(3);
        map.clear();
        map["id"] = "b+b4";
        doc->insertStart("blip", map);
        doc->insertEnd();
        doc->retain(7);
        doc->endDelta();
        wavelet->updateConversation();
        wavelet->print_();

        doc->beginDelta();
        doc->retain(5);
        doc->deleteStart("blip");
        doc->deleteEnd();
        doc->retain(7);
        doc->endDelta();
        wavelet->updateConversation();
        wavelet->print_();

        Blip* b = wavelet->blip("b+b1");
        StructuredDocument* bdoc = b->document();
        DocumentMutation m1;
        // bdoc->beginDelta();
        map.clear();
        map["author"] = "torben.weis@googlewave.com";
        m1.insertStart("contributor", map);
        m1.insertEnd();
        map.clear();
        m1.insertStart("body", map);
        m1.insertStart("line", map);
        m1.insertEnd();
        m1.insertChars("Hello World ");
        QHash<QString,QString> format;
        format["bold"] = "true";
        m1.annotationBoundary(QList<QString>(), format);
        m1.insertChars("this is bold");
        QList<QString> end;
        end.append("bold");
        m1.annotationBoundary(end, QHash<QString,QString>());
        m1.insertChars(" and normal again");
        map.clear();
        m1.insertStart("line", map);
        m1.insertEnd();
        m1.insertChars("Here comes a new line");
        m1.insertEnd();
        // bdoc->endDelta();
        m1.apply(bdoc);
        bdoc->print_();

        // bdoc->beginDelta();
        DocumentMutation m2;
        m2.retain(6);
        m2.insertChars("a");
        m2.retain(5);
        QHash<QString,QString> format2;
        format2["italic"] = "true";
        m2.annotationBoundary(QList<QString>(), format2);
        m2.retain(10);
        QList<QString> end2;
        end2.append("italic");
        m2.annotationBoundary(end2, QHash<QString,QString>());
        m2.retain(49);
        m2.apply(bdoc);
        bdoc->print_();

        b = wavelet->blip("b+b3");
        DocumentMutation m3;
        bdoc = b->document();
        map.clear();
        map["author"] = "torben.weis@googlewave.com";
        m3.insertStart("contributor", map);
        m3.insertEnd();
        map.clear();
        m3.insertStart("body", map);
        m3.insertStart("line", map);
        m3.insertEnd();
        m3.insertEnd();
        m3.apply(bdoc);
        bdoc->print_();

        b = wavelet->blip("b+b4");
        DocumentMutation m4;
        bdoc = b->document();
        map.clear();
        map["author"] = "torben.weis@googlewave.com";
        m4.insertStart("contributor", map);
        m4.insertEnd();
        map.clear();
        m4.insertStart("body", map);
        m4.insertStart("line", map);
        m4.insertEnd();
        m4.insertEnd();
        m4.apply(bdoc);
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

        WaveListView* wlview = new WaveListView(en->inbox());

        splitter->addWidget(wlview);
        splitter->addWidget(view);
        w->setCentralWidget(splitter);
        w->setInboxView(wlview);
        w->setWaveView(view);
        view->connect( wlview, SIGNAL(selected(Wave*)), SLOT(setWave(Wave*)));
        w->show();    
        en->inbox()->addWave(wave);
    }
    // End Test

    //WaveletView* v = new WaveletView(view, wavelet);

    testprotobuf();

    //    return 0;
    return a.exec();
}

