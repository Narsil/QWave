#include "waveurl.h"
#include <QUrl>
#include <QStringList>

WaveUrl::WaveUrl()
{
}

WaveUrl::WaveUrl( const std::string& url )
{
    parse( QString::fromStdString( url) );
}

WaveUrl::WaveUrl( const QString& url )
{
    parse(url);
}

WaveUrl::WaveUrl( const WaveUrl& url )
{
    m_waveDomain = url.m_waveDomain;
    m_waveId = url.m_waveId;
    m_waveletDomain = url.m_waveletDomain;
    m_waveletId = url.m_waveletId;
}

WaveUrl::WaveUrl( const QString& waveDomain, const QString& waveId, const QString& waveletDomain, const QString& waveletId )
    : m_waveDomain(waveDomain), m_waveId(waveId), m_waveletDomain(waveletDomain), m_waveletId(waveletId)
{
}

bool WaveUrl::isNull() const
{
    return m_waveletId.isNull();
}

QString WaveUrl::toString() const
{
    QUrl url;
    url.setScheme("wave");
    url.setHost(m_waveletDomain);
    if ( m_waveletDomain == m_waveDomain)
        url.setPath( "/" + m_waveId + "/" + m_waveletId );
    else
        url.setPath( "/" + m_waveDomain + "$" + m_waveId + "/" + m_waveletId );
    return url.toString();
}

bool WaveUrl::operator==( const WaveUrl& url ) const
{
    return( m_waveDomain == url.m_waveDomain &&
            m_waveId == url.m_waveId &&
            m_waveletDomain == url.m_waveletDomain &&
            m_waveletId == url.m_waveletId );
}

bool WaveUrl::operator!=( const WaveUrl& url ) const
{
    return( m_waveDomain != url.m_waveDomain ||
            m_waveId != url.m_waveId ||
            m_waveletDomain != url.m_waveletDomain ||
            m_waveletId != url.m_waveletId );
}

WaveUrl& WaveUrl::operator=( const WaveUrl& url )
{
    m_waveDomain = url.m_waveDomain;
    m_waveId = url.m_waveId;
    m_waveletDomain = url.m_waveletDomain;
    m_waveletId = url.m_waveletId;
    return *this;
}

WaveUrl& WaveUrl::operator=( const QString& url )
{
    if ( !parse(url) )
    {
        m_waveId = QString::null;
        m_waveDomain = QString::null;
        m_waveletDomain = QString::null;
        m_waveletId = QString::null;
    }
    return *this;
}

bool WaveUrl::parse(const QString& url)
{
    QUrl u( url );
    if ( !u.isValid() || u.scheme() != "wave" || u.host().isEmpty() )
        return false;
    m_waveletDomain = u.host();
    QString path = u.path();
    QStringList lst = path.split('/', QString::SkipEmptyParts );
    if ( lst.count() != 2 )
        return false;
    m_waveId = lst[0];
    m_waveletId = lst[1];
    int index = m_waveId.indexOf('$');
    if ( index != -1 )
    {
        m_waveDomain = m_waveId.left(index);
        m_waveId = m_waveId.mid(index+1);
    }
    else
        m_waveDomain = m_waveletDomain;
    return true;
}
