#include <QtTest/QtTest>
#include <QtGlobal>
#include "app/environment.h"
#include "app/settings.h"
#include "network/networkadapter.h"

 class RemoteOT: public QObject
 {
     Q_OBJECT
 private slots:
     void initTestCase();
     void toUpper();
     void cleanupTestCase();

private:
     Environment* m_environment;
 };

void RemoteOT::toUpper()
{
     QString str = "Hello";
     QVERIFY(str.toUpper() == "HELLO");
}

void RemoteOT::initTestCase()
{
    m_environment = new Environment("remoteot");
    Settings* settings = m_environment->settings();
    settings->setServerName("wave2.vs.uni-due.de");
    settings->setServerPort(9875);
    settings->setUserAddress( "joedoe@" + settings->serverName() );
    settings->setUserName("JoeDoe");
    m_environment->configure();

    // Wait for the connection
    NetworkAdapter* net = m_environment->networkAdapter();
    while( !net->isOnline() && !net->hasConnectionError() )
        QTest::qWait(250);

    if ( net->hasConnectionError() )
        QFAIL("Connection error");
}

void RemoteOT::cleanupTestCase()
{
}

QTEST_MAIN(RemoteOT)
#include "main.moc"
