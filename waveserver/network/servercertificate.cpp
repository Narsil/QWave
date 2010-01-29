#include "servercertificate.h"
#include "app/settings.h"

#include <QFile>

ServerCertificate::ServerCertificate()
{
    QFile file( Settings::settings()->certificateFile() );
    bool ok = file.open( QFile::ReadOnly );
    Q_ASSERT( ok );

    m_certificates = QSslCertificate::fromDevice( &file );
    Q_ASSERT( m_certificates.length() > 0 );
}

QByteArray ServerCertificate::signerInfo() const
{
    // No idea what to do here
    return m_certificates[0].digest( QCryptographicHash::Sha1 ).toBase64();
}

QList<QByteArray> ServerCertificate::toBase64() const
{
    QList<QByteArray> result;
    foreach( QSslCertificate cert, m_certificates )
    {
        result.append( cert.toPem() );
    }
    return result;
}

