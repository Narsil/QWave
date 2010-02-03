#ifndef SERVERCERTIFICATE_H
#define SERVERCERTIFICATE_H

#include <QList>
#include <QByteArray>
#include <QSslCertificate>
#include <openssl/rsa.h>

class ServerCertificate
{
public:
    ~ServerCertificate();

    /**
      * @return the singer info in binary.
      */
    QByteArray signerId() const { return m_signerId; }
    /**
      * @return a list of Base64 encoded certificates.
      */
    QList<QByteArray> toBase64() const;

    bool verify( const QByteArray& data, const QByteArray& signature ) const;

    virtual bool isValid() const { return m_certificates.count() > 0 && m_publicKey != 0; }

protected:
    ServerCertificate();

    void setPEMCertificates( const QList<QByteArray>& certificates );
    void setPEMCertificates( const QByteArray& certificates );

private:
    void computeSignerInfo();
    bool computePublicKey();

    /**
      * The base 64 encoded signer id.
      */
    QByteArray m_signerId;
    QList<QSslCertificate> m_certificates;
    RSA* m_publicKey;
};

/**
  * A local server certificate contains in addition the private key and
  * is therfore able to sign messages.
  */
class LocalServerCertificate : public ServerCertificate
{
public:
    ~LocalServerCertificate();

    /**
      * Sign a message.
      *
      * @return the signature in binary format.
      */
    QByteArray sign( const QByteArray& data ) const;

    /**
      * The certificate used by the local server
      */
    static LocalServerCertificate* certificate();

private:
    LocalServerCertificate();

    RSA* m_privateKey;

    static LocalServerCertificate* s_certificate;
};

class RemoteServerCertificate : public ServerCertificate
{
public:    
    RemoteServerCertificate(const QList<QByteArray> certificateChain );
    ~RemoteServerCertificate();
};

#endif // SERVERCERTIFICATE_H
