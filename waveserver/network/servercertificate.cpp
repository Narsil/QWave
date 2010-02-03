#include "servercertificate.h"
#include "app/settings.h"

#include <QFile>
#include <QSslKey>
#include <QCryptographicHash>

#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>

ServerCertificate::ServerCertificate()
        : m_signerId( 32, 0 ), m_publicKey(0)
{
}

ServerCertificate::~ServerCertificate()
{
    if ( m_publicKey )
        RSA_free(m_publicKey);
}

bool ServerCertificate::computePublicKey()
{
    if ( m_certificates.count() == 0 )
        return false;

    QByteArray ba = m_certificates[0].publicKey().toPem();
    BIO *pub_bio = BIO_new_mem_buf(ba.data(), ba.length());
    if( pub_bio == 0 )
    {
        ERR_print_errors_fp(stdout);
        return false;
    }

    m_publicKey = PEM_read_bio_RSA_PUBKEY(pub_bio, NULL, NULL, NULL);
    if( m_publicKey == 0)
    {
        BIO_free(pub_bio);
        ERR_print_errors_fp(stdout);
        return false;
    }

    BIO_free(pub_bio);

    return true;
}

void ServerCertificate::computeSignerInfo()
{
    if ( !isValid() )
        return;

    int len = 0;
    // Get a sequence of DER encoded certificates
    QList<QByteArray> certs;
    foreach( QSslCertificate c, m_certificates )
    {
        QByteArray cba = c.toDer();
        len += cba.length();
        certs.append( cba );
    }

    Q_ASSERT( len < 0xFFFF );

    // Construct an ASN.1 sequence of certificates
    QByteArray seq( len + 3 + (len >= 256 ? 1 : 0), 0 );
    unsigned char* ptr = (unsigned char*) seq.data();
    ptr[0] = 0b110000;
    int offset = 4;
    if ( len < 256 )
    {
        ptr[1] = 0b10000001;
        ptr[2] = len;
        offset = 3;
    }
    else
    {
        ptr[1] = 0b10000010;
        ptr[2] = (( len & 0xFF00 ) >> 8);
        ptr[3] = ( len & 0xFF );
    }
    // Copy the certificates
    for( int k = certs.length() - 1; k >= 0; --k )
    {
        QByteArray ba = certs[k];
        for( int i = 0; i < ba.length(); ++i )
        {
            seq.data()[offset++] = ba.constData()[i];
        }
    }

    SHA256( (const unsigned char*)seq.data(), seq.length(), (unsigned char*)m_signerId.data() );
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

void ServerCertificate::setPEMCertificates( const QList<QByteArray>& certificates )
{
    m_certificates.clear();
    foreach( const QByteArray& ba, certificates )
    {
        QSslCertificate c( ba, QSsl::Pem );
        if ( !c.isValid() )
        {
            qDebug("Error reading PEM certificate %s", ba.constData());
            m_certificates.clear();
            return;
        }
        m_certificates.append( c );
    }
    computePublicKey();
    computeSignerInfo();
}

void ServerCertificate::setPEMCertificates( const QByteArray& certificates )
{
    m_certificates = QSslCertificate::fromData( certificates, QSsl::Pem );
    foreach( const QSslCertificate& c, m_certificates )
    {
        if ( !c.isValid() )
        {
            qDebug("Error reading PEM certificate");
            m_certificates.clear();
            return;
        }
    }

    computePublicKey();
    computeSignerInfo();
}



LocalServerCertificate* LocalServerCertificate::s_certificate = 0;

LocalServerCertificate::LocalServerCertificate()
        : ServerCertificate(), m_privateKey(0)
{
    // Load the certificate
    QFile file( Settings::settings()->certificateFile() );
    bool ok = file.open( QFile::ReadOnly );
    if ( !ok )
    {
        qDebug("Could not load certificate file");
        Q_ASSERT( false );
        return;
    }

    QByteArray bacert = file.readAll();
    file.close();

    setPEMCertificates( bacert );

    // Load the private Key
    QFile file2( Settings::settings()->privateKeyFile() );
    ok = file2.open( QFile::ReadOnly );
    if ( !ok )
    {
        qDebug("Could not load private key file");
        Q_ASSERT( false );
        return;
    }
    QByteArray ba = file2.readAll();
    file2.close();

    // Create a private key
    BIO *priv_bio = BIO_new_mem_buf(ba.data(), -1);
    if( priv_bio == 0 )
    {
        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return;
    }

    m_privateKey = PEM_read_bio_RSAPrivateKey(priv_bio, NULL, NULL, NULL);
    if( m_privateKey == 0)
    {
        BIO_free(priv_bio);

        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return;
    }

    BIO_free(priv_bio);
}

LocalServerCertificate::~LocalServerCertificate()
{
    if ( m_privateKey )
        RSA_free(m_privateKey);
}

QByteArray LocalServerCertificate::sign( const QByteArray& message ) const
{
    Q_ASSERT( m_privateKey );

    QByteArray hash = QCryptographicHash::hash( message, QCryptographicHash::Sha1 );
    QByteArray ba;
    ba.resize( RSA_size(m_privateKey) );
    unsigned char* signature = (unsigned char*)ba.data();
    unsigned int slen = 0;
    if( RSA_sign(NID_sha1, (unsigned char*) hash.constData(), hash.length(), signature, &slen, m_privateKey) != 1)
    {
        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return QByteArray();
    }

    return ba;
}

LocalServerCertificate* LocalServerCertificate::certificate()
{
    if ( s_certificate == 0 )
        s_certificate = new LocalServerCertificate();
    return s_certificate;
}



RemoteServerCertificate::RemoteServerCertificate(const QList<QByteArray> certificateChain )
{
    setPEMCertificates( certificateChain );
}

RemoteServerCertificate::~RemoteServerCertificate()
{
}
