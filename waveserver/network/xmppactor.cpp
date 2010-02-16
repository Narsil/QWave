#include "xmppactor.h"
#include "xmppvirtualconnection.h"
#include "actor/actorid.h"

#include <QtGlobal>
#include <QDateTime>

qint64 XmppActor::s_id = 0;

XmppActor::XmppActor(XmppVirtualConnection* con)
        : Actor(), m_connection(con), m_actorId( con, QString::number( s_id++ ) )
{
}

void XmppActor::log( const char* error, const char* file, int line )
{
    QString d = connection()->domain();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("INFO in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void XmppActor::log( const QString& error, const char* file, int line )
{
    log( error.toAscii().constData(), file, line );
}

void XmppActor::logErr( const char* error, const char* file, int line )
{
    QString d = connection()->domain();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("ERROR in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void XmppActor::logErr( const QString& error, const char* file, int line )
{
    logErr( error.toAscii().constData(), file, line );
}
