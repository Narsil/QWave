#include <QtTest/QtTest>
#include <QtGlobal>
#include <QHash>

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
    void toUpper();
    void concurrentEdit();
    void cleanupTestCase();

private:
    Environment* m_environment1;
    Environment* m_environment2;
 };

void RemoteOT::toUpper()
{
     QString str = "Hello";
     QVERIFY(str.toUpper() == "HELLO");
}

void RemoteOT::initTestCase()
{
    m_environment1 = new Environment("remoteot1");
    Settings* settings = m_environment1->settings();
    settings->setServerName("localhost");
    settings->setServerPort(9876);
    settings->setUserAddress( "joedoe@" + settings->serverName() );
    settings->setUserName("JoeDoe");
    m_environment1->configure();

    // Wait for the connection
    NetworkAdapter* net = m_environment1->networkAdapter();
    while( !net->isOnline() && !net->hasConnectionError() )
        QTest::qWait(250);

    if ( net->hasConnectionError() )
        QFAIL("Connection error");

    m_environment2 = new Environment("remoteot2");
    settings = m_environment2->settings();
    settings->setServerName("localhost");
    settings->setServerPort(9876);
    settings->setUserAddress( "jane@" + settings->serverName() );
    settings->setUserName("Jane");
    m_environment2->configure();

    // Wait for the connection
    net = m_environment2->networkAdapter();
    while( !net->isOnline() && !net->hasConnectionError() )
        QTest::qWait(250);

    if ( net->hasConnectionError() )
        QFAIL("Connection error");

    // Create a wave on behalf of user 1
    Wave* wave1 = m_environment1->createWave( m_environment1->networkAdapter()->serverName(), "w+testwave");
    // Add the new wave to the inbox.
    m_environment1->inbox()->addWave(wave1);
    // Get the root wavelet
    Wavelet* wavelet1 = wave1->wavelet();
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
    map["id"] = "b+b1";
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
    wavelet1->processor()->handleSend( m2, "b+b1" );

    // Wait until this has been processed
    while( wavelet1->processor()->queuedDeltaCount() > 0 )
        QTest::qWait(250);

    // Open the wave for user 2
    Wave* wave2 = m_environment2->createWave( m_environment1->networkAdapter()->serverName(), "w+testwave");
    // Add the new wave to the inbox.
    m_environment2->inbox()->addWave(wave2);
    // Get the root wavelet
    Wavelet* wavelet2 = wave2->wavelet();
    // Open the wavelet and receive the initial deltas
    m_environment2->networkAdapter()->openWavelet(wavelet2);

    // Wait until user 2 got all deltas
    while( wavelet2->processor()->serverVersion() < 4 )
        QTest::qWait(250);
}

void RemoteOT::concurrentEdit()
{
    Wavelet* wavelet1 = m_environment1->wave(m_environment1->networkAdapter()->serverName(), "w+testwave")->wavelet();
    // Create an empty blip
    DocumentMutation m1;
    m1.retain(5);
    m1.insertChars("Hallo");
    m1.retain(1);
    wavelet1->processor()->setSuspendSending(true);
    wavelet1->processor()->handleSend( m1, "b+b1" );

    Wavelet* wavelet2 = m_environment2->wave(m_environment1->networkAdapter()->serverName(), "w+testwave")->wavelet();
    // Create an empty blip
    DocumentMutation m2;
    m2.retain(5);
    m2.insertChars("Welt");
    m2.retain(1);
    wavelet2->processor()->setSuspendSending(true);
    wavelet2->processor()->handleSend( m2, "b+b1" );

    wavelet1->processor()->setSuspendSending(false);
    // Wait until this has been processed
    while( wavelet1->processor()->queuedDeltaCount() > 0 )
        QTest::qWait(250);

    // Wait until user 2 got all deltas
    while( wavelet2->processor()->serverVersion() < 5 )
        QTest::qWait(250);

    wavelet2->processor()->setSuspendSending(false);

    // Wait until user 1 and 2 got all deltas
    while( wavelet1->processor()->serverVersion() < 6 )
        QTest::qWait(250);
    while( wavelet2->processor()->serverVersion() < 6 )
        QTest::qWait(250);

    wavelet1->rootBlips()[0]->document()->print_();
    wavelet2->rootBlips()[0]->document()->print_();

    // Now compare the document of both users. It must be the same
    QCOMPARE( wavelet1->rootBlips()[0]->document()->toString(), wavelet2->rootBlips()[0]->document()->toString() );
}

void RemoteOT::cleanupTestCase()
{
}

QTEST_MAIN(RemoteOT)
#include "main.moc"
