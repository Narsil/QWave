#include <QtTest/QtTest>
#include <QtGlobal>
#include <QHash>
#include <QDateTime>

#include "app/environment.h"
#include "app/settings.h"
#include "network/networkadapter.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/blipdocument.h"
#include "model/otprocessor.h"
#include "model/structureddocument.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "model/participant.h"

 class RemoteOT: public QObject
 {
     Q_OBJECT
 private slots:
    void initTestCase();
    void concurrentEdit();
    void concurrentEdit2();
    void concurrentEdit3();
    void concurrentEdit4();
    void cleanupTestCase();

private:
    Settings* settings;
    Environment* m_environment1;
    Environment* m_environment2;
    // Used to make blip names unique
    QString m_rand;
    // The current wave name. Has the form: "w+xxxxxxx".
    QString m_wavename;
 };

void RemoteOT::initTestCase()
{
    // Create a random name for use in the blip
    m_rand = QString("b%1b").arg( QDateTime::currentDateTime().toTime_t() );
    m_wavename = QString("w+%1").arg( QDateTime::currentDateTime().toTime_t() );

    // Connect JoeDoe to the server
    m_environment1 = new Environment("remoteot1");
    settings = m_environment1->settings();
    settings->setUserAddress( "joedoe@" + settings->waveDomain() );
    settings->setUserName("JoeDoe");
    m_environment1->configure();

    // Wait for the connection
    NetworkAdapter* net = m_environment1->networkAdapter();
    while( !net->isOnline() && !net->hasConnectionError() )
        QTest::qWait(250);

    if ( net->hasConnectionError() )
        QFAIL("Connection error");

    // Connect Jane to the server
    m_environment2 = new Environment("remoteot2");
    settings = m_environment2->settings();
    settings->setUserAddress( "jane@" + settings->waveDomain() );
    settings->setUserName("Jane");
    m_environment2->configure();

    // Wait for the connection
    net = m_environment2->networkAdapter();
    while( !net->isOnline() && !net->hasConnectionError() )
        QTest::qWait(250);

    if ( net->hasConnectionError() )
        QFAIL("Connection error");

    // Create a wave on behalf of user 1
    Wave* wave1 = m_environment1->createWave( settings->waveDomain(), m_wavename);
    // Add the new wave to the inbox.
    m_environment1->inbox()->addWave(wave1);
    // Get the root wavelet
    Wavelet* wavelet1 = wave1->wavelet();
    //Turn off the gathering flag of this processor
    wavelet1->processor()->setGatheringDeltas(false);
    // Tell the server about the new wave
    wavelet1->processor()->handleSendAddParticipant(m_environment1->localUser());
    // Add user 2 to the wave
    wavelet1->processor()->handleSendAddParticipant(m_environment2->localUser());
    // Open the wavelet to receive all deltas
    m_environment1->networkAdapter()->openWavelet(wavelet1);

    // Create a conversation
    DocumentMutation m1;
    m1.insertStart("conversation");
    QHash<QString,QString> map;
    map["id"] = "b+" + m_rand + "1";
    m1.insertStart("blip", map);
    m1.insertEnd();
    map.clear();
    map["id"] = "b+" + m_rand + "2";
    m1.insertStart("blip", map);
    m1.insertEnd();
    map.clear();
    map["id"] = "b+" + m_rand + "3";
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.insertEnd();
    wavelet1->processor()->handleSend( m1, "conversation" );

    // Create an empty blip
    DocumentMutation m2;
    map.clear();
    map["name"] = m_environment1->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet1->processor()->handleSend( m2, "b+" + m_rand + "1" );

    // Create an empty blip
    DocumentMutation m3;
    map.clear();
    map["name"] = m_environment1->localUser()->address();
    m3.insertStart("contributor", map);
    m3.insertEnd();
    map.clear();
    m3.insertStart("body", map);
    m3.insertStart("line", map);
    m3.insertEnd();
    m3.insertEnd();
    wavelet1->processor()->handleSend( m3, "b+" + m_rand + "2" );

    // Create an empty blip
    DocumentMutation m4;
    map.clear();
    map["name"] = m_environment1->localUser()->address();
    m4.insertStart("contributor", map);
    m4.insertEnd();
    map.clear();
    m4.insertStart("body", map);
    m4.insertStart("line", map);
    m4.insertEnd();
    m4.insertEnd();
    wavelet1->processor()->handleSend( m4, "b+" + m_rand + "3" );

    // Wait until this has been processed
    while( wavelet1->processor()->queuedDeltaCount() > 0 )
        QTest::qWait(250);

    // Open the wave for user 2
    Wave* wave2 = m_environment2->createWave( settings->waveDomain(), m_wavename);
    // Add the new wave to the inbox.
    m_environment2->inbox()->addWave(wave2);
    // Get the root wavelet
    Wavelet* wavelet2 = wave2->wavelet();
    //Turn off the gathering flag of this processor
    wavelet2->processor()->setGatheringDeltas(false);
    // Open the wavelet and receive the initial deltas
    m_environment2->networkAdapter()->openWavelet(wavelet2);

    // Wait until user 2 got all deltas
    while( wavelet2->processor()->serverVersion() < 4 )
        QTest::qWait(250);
}

void RemoteOT::concurrentEdit()
{
    // Get the wavelet for user1 we just created
    Wavelet* wavelet1 = m_environment1->wave(settings->waveDomain(), m_wavename)->wavelet();
    // Write 'Hallo' to the blip
    DocumentMutation m1;
    m1.retain(5);
    m1.insertChars("Hallo");
    m1.retain(1);
    // Do not send the delta to the server yet
    wavelet1->processor()->setSuspendSending(true);
    wavelet1->processor()->handleSend( m1, "b+" + m_rand + "1" );

    // Get the same wavelet for user2
    Wavelet* wavelet2 = m_environment2->wave(settings->waveDomain(), m_wavename)->wavelet();
    // Concurrently write 'Welt' to the same blip
    DocumentMutation m2;
    m2.retain(5);
    m2.insertChars("Welt");
    m2.retain(1);
    // Do not send the delta to the server yet
    wavelet2->processor()->setSuspendSending(true);
    wavelet2->processor()->handleSend( m2, "b+" + m_rand + "1" );

    // Let client1 send its delta to the server
    wavelet1->processor()->setSuspendSending(false);
    // Wait until this has been processed
    while( wavelet1->processor()->queuedDeltaCount() > 0 )
        QTest::qWait(250);

    // Wait until user 2 got all deltas
    while( wavelet2->processor()->serverVersion() < 7 )
        QTest::qWait(250);

    // User 2 will have transformed its delta. Now send the transformed delta to the server.
    wavelet2->processor()->setSuspendSending(false);

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < 8 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < 8 )
        QTest::qWait(250);

    // Both clients should now have the same blip.
    wavelet1->rootBlips()[0]->document()->print_();
    wavelet2->rootBlips()[0]->document()->print_();

    // Now compare the document of both users. It must be the same
    QCOMPARE( wavelet1->rootBlips()[0]->document()->toString(), wavelet2->rootBlips()[0]->document()->toString() );
}

void RemoteOT::concurrentEdit2()
{
    // Get the wavelets for both users
    Wavelet* wavelet1 = m_environment1->wave(settings->waveDomain(), m_wavename)->wavelet();
    Wavelet* wavelet2 = m_environment2->wave(settings->waveDomain(), m_wavename)->wavelet();

    int v1 = wavelet1->processor()->serverVersion();
    int v2 = wavelet2->processor()->serverVersion();

    // Insert "Hallo" in the blip
    DocumentMutation m1;
    m1.retain(5);
    m1.insertChars("Hallo");
    m1.retain(1);
    wavelet1->processor()->handleSend( m1, "b+" + m_rand + "2" );

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < v1 + 1 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < v2 + 1 )
        QTest::qWait(250);

    // User1 deleta 'll' from 'Hallo' leading to 'Hao'
    DocumentMutation m1b;
    m1b.retain(5+2);
    m1b.deleteChars("ll");
    m1b.retain(2);
    wavelet1->processor()->handleSend( m1b, "b+" + m_rand + "2" );

    // User2 inserts 'XZY' in the middle of the two 'll' leading to 'HalXZYlo'
    DocumentMutation m2;
    m2.retain(5 + 2 + 1);
    m2.insertChars("XZY");
    m2.retain(3);
    wavelet2->processor()->handleSend( m2, "b+" + m_rand + "2" );

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < v1 + 3 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < v1 + 3 )
        QTest::qWait(250);

    // Both users should now have the same document, in particular 'HaXZYo'
    wavelet1->rootBlips()[1]->document()->print_();
    wavelet2->rootBlips()[1]->document()->print_();

    // Now compare the document of both users. It must be the same
    QCOMPARE( wavelet1->rootBlips()[1]->document()->toString(), wavelet2->rootBlips()[1]->document()->toString() );
}

void RemoteOT::concurrentEdit3()
{
    // Get the wavelets for both users
    Wavelet* wavelet1 = m_environment1->wave(settings->waveDomain(), m_wavename)->wavelet();
    Wavelet* wavelet2 = m_environment2->wave(settings->waveDomain(), m_wavename)->wavelet();

    int v1 = wavelet1->processor()->serverVersion();
    int v2 = wavelet2->processor()->serverVersion();

    // Insert "Hallo" in the blip
    DocumentMutation m1;
    m1.retain(5);
    m1.insertChars("Hallo");
    m1.retain(1);
    wavelet1->processor()->handleSend( m1, "b+" + m_rand + "3" );

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < v1 + 1 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < v2 + 1 )
        QTest::qWait(250);

    // Format the entire 'Hallo' with font-size 20
    DocumentMutation m1b;
    m1b.retain(5);
    StructuredDocument::AnnotationChange format;
    format["style/fontSize"].second = "20";
    m1b.annotationBoundary(QList<QString>(), format);
    m1b.retain(5);
    QList<QString> end;
    end.append("style/fontSize");
    m1b.annotationBoundary(end, StructuredDocument::AnnotationChange());
    m1b.retain(1);
    wavelet1->processor()->handleSend( m1b, "b+" + m_rand + "3" );

    // Between the 'll' insert 'ZWOELF' and set its font size to 12
    DocumentMutation m2;
    m2.retain(5 + 3);
    StructuredDocument::AnnotationChange format2;
    format2["style/fontSize"].second = "12";
    m2.annotationBoundary(QList<QString>(), format2);
    m2.insertChars("ZWOELF");
    m2.annotationBoundary(end, StructuredDocument::AnnotationChange());
    m2.retain(3);
    wavelet2->processor()->handleSend( m2, "b+" + m_rand + "3" );

    // Show how the code transforms
    bool ok = false;
    QPair<DocumentMutation,DocumentMutation> pair = DocumentMutation::xform( m1b, m2, &ok );
    QVERIFY(ok == true);
    pair.first.print_();
    pair.second.print_();

    // Show how the code transforms
    ok = false;
    pair = DocumentMutation::xform( m2, m1b, &ok );
    QVERIFY(ok == true);
    pair.first.print_();
    pair.second.print_();

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < v1 + 3 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < v1 + 3 )
        QTest::qWait(250);

    // Now both users should see the same document
    wavelet1->rootBlips()[2]->document()->print_();
    wavelet2->rootBlips()[2]->document()->print_();

    // Now compare the document of both users. It must be the same
    QCOMPARE( wavelet1->rootBlips()[2]->document()->toString(), wavelet2->rootBlips()[2]->document()->toString() );
}

void RemoteOT::concurrentEdit4()
{
    // Get the wavelets for both users
    Wavelet* wavelet1 = m_environment1->wave(settings->waveDomain(), m_wavename)->wavelet();
    Wavelet* wavelet2 = m_environment2->wave(settings->waveDomain(), m_wavename)->wavelet();

    int v1 = wavelet1->processor()->serverVersion();
    int v2 = wavelet2->processor()->serverVersion();

    // Insert "Test1" in the blip1
    DocumentMutation m1;
    m1.retain(5);
    m1.insertChars("Test1");
    m1.retain(10);
    wavelet1->processor()->setSuspendSending(true);
    wavelet1->processor()->handleSend( m1, "b+" + m_rand + "1" );

    // Insert "Test2" in the blip2
    DocumentMutation m2;
    m2.retain(5);
    m2.insertChars("Test2");
    m2.retain(7);
    wavelet2->processor()->setSuspendSending(true);
    wavelet2->processor()->handleSend( m2, "b+" + m_rand + "2" );

    wavelet1->processor()->setSuspendSending(false);
    wavelet2->processor()->setSuspendSending(false);

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < v1 + 2 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < v2 + 2 )
        QTest::qWait(250);

    // Now both users should see the same document
    wavelet1->rootBlips()[0]->document()->print_();
    wavelet2->rootBlips()[1]->document()->print_();

    // Now compare the document of both users. It must be the same
    QCOMPARE( wavelet1->rootBlips()[0]->document()->toPlainText().left(5), QString("Test1") );
    QCOMPARE( wavelet1->rootBlips()[1]->document()->toPlainText().left(5), QString("Test2") );
}

void RemoteOT::cleanupTestCase()
{
}

QTEST_MAIN(RemoteOT)
#include "main.moc"
