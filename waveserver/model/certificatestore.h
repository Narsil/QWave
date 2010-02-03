#ifndef CERTIFICATESTORE_H
#define CERTIFICATESTORE_H

#include <QHash>
#include <QByteArray>

class ServerCertificate;
class RemoteServerCertificate;

class CertificateStore
{
public:
    ~CertificateStore();
    const ServerCertificate* certificate( const QByteArray& signerId );
    /**
      * The store takes over ownership of this certificate.
      */
    void addCertificate( RemoteServerCertificate* certificate );

    static CertificateStore* store();

private:
    CertificateStore();

    QHash<QByteArray,RemoteServerCertificate*> m_certificates;

    static CertificateStore* s_store;
};

#endif // CERTIFICATESTORE_H
