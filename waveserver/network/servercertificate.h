#ifndef SERVERCERTIFICATE_H
#define SERVERCERTIFICATE_H

#include <QList>
#include <QSslCertificate>
#include <openssl/rsa.h>

class ServerCertificate
{
public:
    ServerCertificate();
    ~ServerCertificate();

    QByteArray signerInfo() const;
    QList<QByteArray> toBase64() const;

    QByteArray sign( const QByteArray& data ) const;

    static ServerCertificate* certificate();

private:
    QList<QSslCertificate> m_certificates;
    RSA* m_publicKey;
    RSA* m_privateKey;
    QByteArray m_signerInfo;
    bool m_hasSignerInfo;

    static ServerCertificate* s_certificate;
};

#endif // SERVERCERTIFICATE_H
