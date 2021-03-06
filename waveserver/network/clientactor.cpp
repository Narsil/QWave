#include "clientactor.h"
#include "clientconnection.h"
#include "fcgi/fcgiclientconnection.h"
#include <QString>
#include <QDateTime>
#include <QtGlobal>

quint64 ClientActor::s_id = 0;

ClientActor::ClientActor( ClientConnection* con )
    : Actor( QString::number( s_id++ ), con ), m_pbConnection( con ), m_fcgiConnection(0)
{
}

ClientActor::ClientActor( FCGIClientConnection* con )
    : Actor( QString::number( s_id++ ), con ), m_pbConnection(0), m_fcgiConnection( con )
{
}

void ClientActor::log( const char* error, const char* file, int line )
{
    QString d = participant();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("INFO in %s:%i talking to %s on behalf of %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void ClientActor::log( const QString& error, const char* file, int line )
{
    log( error.toAscii().constData(), file, line );
}

void ClientActor::logErr( const char* error, const char* file, int line )
{
    QString d = participant();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("ERROR in %s:%i talking on behalf of %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void ClientActor::logErr( const QString& error, const char* file, int line )
{
    logErr( error.toAscii().constData(), file, line );
}

QString ClientActor::participant() const
{
    if ( m_pbConnection )
        return m_pbConnection->participant();
    return m_fcgiConnection->participant();
}

ActorGroup* ClientActor::connection()
{
    if ( m_pbConnection )
        return m_pbConnection;
    return m_fcgiConnection;
}
