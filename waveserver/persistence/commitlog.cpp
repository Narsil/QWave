#include "commitlog.h"
#include "app/settings.h"
#include "persistence/store.h"

CommitLog::CommitLog(Store* store)
        : QObject( store )
{
    m_file.setFileName( Settings::settings()->logFile() );
    bool ok = m_file.open( QFile::ReadWrite );
    // TODO: Better error handling
    Q_ASSERT(ok);
}

CommitLog::~CommitLog()
{
    close();
}

QByteArray CommitLog::read()
{
    if ( !m_file.isOpen() )
        return false;

    int len;
    if ( m_file.read( (char*)&len, sizeof(len) ) != sizeof(len) )
        return QByteArray();

    QByteArray result( len, 0 );
    if ( m_file.read( result.data(), len ) != len )
        return QByteArray();

    return result;
}

bool CommitLog::write( const QByteArray& data )
{
    if ( !m_file.isOpen() )
        return false;
    if ( data.isEmpty() )
        return false;

    int len = data.length();
    qint64 res = m_file.write( (const char*)(&len), sizeof(len) );
    if ( res != sizeof(len) )
        return false;

    res = m_file.write( data.constData(), data.length() );
    if ( res != data.length() )
        return false;

    m_file.flush();

    return true;
}

void CommitLog::close()
{
    m_file.close();
}
