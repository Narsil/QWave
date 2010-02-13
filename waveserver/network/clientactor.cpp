#include "clientactor.h"
#include "clientconnection.h"
#include "model/participant.h"
#include <QString>
#include <QDateTime>
#include <QtGlobal>

quint64 ClientActor::s_id = 0;

ClientActor::ClientActor( ClientConnection* con )
    : m_connection( con ), m_actorId( ActorId::Client, con->id(), QString::number( s_id++ ) )
{
    setObjectName( m_actorId.actor() );
}

void ClientActor::log( const char* error, const char* file, int line )
{
    QString d = "(unknown)";
    if ( connection()->participant() )
        d = connection()->participant()->toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("INFO in %s:%i talking to %s on behalf of %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void ClientActor::log( const QString& error, const char* file, int line )
{
    log( error.toAscii().constData(), file, line );
}

void ClientActor::logErr( const char* error, const char* file, int line )
{
    QString d = "(unknown)";
    if ( connection()->participant() )
        d = connection()->participant()->toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("ERROR in %s:%i talking on behalf of %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void ClientActor::logErr( const QString& error, const char* file, int line )
{
    logErr( error.toAscii().constData(), file, line );
}

