#ifndef WAVEURL_H
#define WAVEURL_H

#include <QString>

class WaveUrl
{
public:
    WaveUrl();
    WaveUrl( const QString& url );
    WaveUrl( const WaveUrl& url );
    WaveUrl( const QString& waveDomain, const QString& waveId, const QString& waveletDomain, const QString& waveletId );

    bool isNull() const;

    QString toString() const;

    bool operator==( const WaveUrl& url ) const;
    bool operator!=( const WaveUrl& url ) const;
    WaveUrl& operator=( const WaveUrl& url );
    WaveUrl& operator=( const QString& url );

    QString waveDomain() const { return m_waveDomain; }
    void setWaveDomain( const QString& domain ) { m_waveDomain = domain; }
    QString waveId() const { return m_waveId; }
    void setWaveId( const QString& id ) { m_waveId = id; }
    QString waveletDomain() const { return m_waveletDomain; }
    void setWaveletDomain( const QString& domain ) { m_waveletDomain = domain; }
    QString waveletId() const { return m_waveletId; }
    void setWaveletId( const QString& id ) { m_waveletId = id; }

private:
    bool parse(const QString& url);

    QString m_waveDomain;
    QString m_waveId;
    QString m_waveletDomain;
    QString m_waveletId;
};

#endif // WAVEURL_H
