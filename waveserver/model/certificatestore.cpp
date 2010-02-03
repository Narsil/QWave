#include "certificatestore.h"
#include "network/servercertificate.h"

CertificateStore* CertificateStore::s_store = 0;

CertificateStore::CertificateStore()
{
}

CertificateStore::~CertificateStore()
{
}

const ServerCertificate* CertificateStore::certificate( const QByteArray& signerId )
{
    if ( signerId == LocalServerCertificate::certificate()->signerId() )
        return LocalServerCertificate::certificate();
    return m_certificates[signerId];
}

void CertificateStore::addCertificate( RemoteServerCertificate* certificate )
{
    QByteArray signerId = certificate->signerId();
    if ( signerId.isEmpty() )
        return;

    if ( m_certificates.contains( signerId ) )
        delete m_certificates[signerId];
    m_certificates.insert( signerId, certificate );
}

CertificateStore* CertificateStore::store()
{
    if ( s_store == 0 )
        s_store = new CertificateStore();
    return s_store;
}
