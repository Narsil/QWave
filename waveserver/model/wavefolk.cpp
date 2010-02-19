#include "wavefolk.h"
#include "model/wave.h"
#include "model/waveurl.h"

WaveFolk* WaveFolk::s_folk = 0;

WaveFolk::WaveFolk(QObject* parent)
        : ActorFolk( "wave", parent )
{
    setHierarchical( true );
}

ActorGroup* WaveFolk::group( const QString& groupId, bool createOnDemand )
{
    Wave* wave = m_waves[ groupId ];
    if ( wave || !createOnDemand )
        return wave;

    int index = groupId.indexOf('$');
    if ( index == -1 )
    {
        qDebug("Malformed wave group ID, $ is missing");
        return 0;
    }
    QString domain = groupId.left(index);
    QString id = groupId.mid(index+1);

    wave = new Wave( this, domain, id );
    m_waves[ groupId ] = wave;
    return wave;
}

Wave* WaveFolk::wave( const QString& domain, const QString& id, bool createOnDemand )
{
    QString groupId = domain + "$" + id;
    Wave* wave = m_waves[ groupId ];
    if ( wave || !createOnDemand )
        return wave;

    wave = new Wave( this, domain, id );
    m_waves[ groupId ] = wave;
    return wave;
}

WaveFolk* WaveFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new WaveFolk();
    return s_folk;
}

ActorId WaveFolk::actorId( const WaveUrl& url )
{
    return ActorId( "wave", url.waveDomain() + "$" + url.waveId() + "/" + url.waveletDomain() + "$" + url.waveletId() );
}
