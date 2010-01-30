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
        : m_publicKey(0), m_privateKey(0), m_signerInfo( 32, 0 )
{
    QFile file( Settings::settings()->certificateFile() );
    bool ok = file.open( QFile::ReadOnly );
    Q_ASSERT( ok );

    m_certificates = QSslCertificate::fromDevice( &file );
    Q_ASSERT( m_certificates.length() > 0 );
    file.close();

    QFile file2( Settings::settings()->privateKeyFile() );
    ok = file2.open( QFile::ReadOnly );
    Q_ASSERT( ok );
    QByteArray ba = file2.readAll();
    file2.close();

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
        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return;
    }

    ba = m_certificates[0].publicKey().toPem();
    BIO *pub_bio = BIO_new_mem_buf(ba.data(), -1);
    if( pub_bio == 0 )
    {
        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return;
    }

    m_publicKey = PEM_read_bio_RSA_PUBKEY(pub_bio, NULL, NULL, NULL);
    if( m_publicKey == 0)
    {
        ERR_print_errors_fp(stdout);
        Q_ASSERT(false);
        return;
    }
}

ServerCertificate::~ServerCertificate()
{
    if ( m_privateKey )
        RSA_free(m_privateKey);
    if ( m_publicKey )
        RSA_free(m_publicKey);
}

QByteArray ServerCertificate::signerInfo() const
{
    if ( !m_signerInfo.isNull() )
        return m_signerInfo;

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

    SHA256( (const unsigned char*)seq.data(), seq.length(), (unsigned char*)m_signerInfo.constData() );

    return m_signerInfo;
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

QByteArray ServerCertificate::sign( const QByteArray& message ) const
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
