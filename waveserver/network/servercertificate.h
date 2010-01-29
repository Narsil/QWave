#ifndef SERVERCERTIFICATE_H
#define SERVERCERTIFICATE_H

#include <QList>
#include <QSslCertificate>

class ServerCertificate
{
public:
    ServerCertificate();

    QByteArray signerInfo() const;
    QList<QByteArray> toBase64() const;

private:
    QList<QSslCertificate> m_certificates;
};

#endif // SERVERCERTIFICATE_H
