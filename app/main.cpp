#include <QtGui/QApplication>
#include <QGraphicsView>
#include "mainwindow.h"

#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blipthread.h"
#include "model/blip.h"
#include "model/synchronizeddocument.h"
#include "view/waveview.h"
#include "view/waveletview.h"
#include "model/documentmutation.h"
#include "model/participant.h"
#include "app/environment.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w1;
    MainWindow w2;

    for( int i = 0; i < 2; ++i )
    {
        MainWindow* w = &w1;
        if ( i == 1 )
            w = &w2;

        // Test
        Environment* en = new Environment("torben.weis@googlewave.com", "torben");

        Participant* p2 = new Participant("joe@acme.com");
        p2->setName("Joe");
        Participant* p3 = new Participant("pam@foobar.com");
        p3->setName("Pam");
        p3->setPixmap( QPixmap("images/user3.jpg") );

        Wave* wave = new Wave(en, "localhost", "conv+12345");
        Wavelet* wavelet = new Wavelet(wave, "localhost", "w+abcdef");
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

        WaveView* view = new WaveView(wave);
        w->setCentralWidget(view);
        w->show();
    }
    // End Test

    //WaveletView* v = new WaveletView(view, wavelet);

    //    return 0;
    return a.exec();
}
