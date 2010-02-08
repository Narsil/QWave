#include "xmppactor.h"
#include "xmppvirtualconnection.h"

#include <QtGlobal>
#include <QDateTime>

XmppActor::XmppActor(XmppVirtualConnection* con)
        : Actor(), m_connection(con)
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
