#include "actorid.h"
#include <QUrl>

ActorId::ActorId(Folk folk, const QString& group, const QString& actor)
        : m_folk(folk), m_group(group), m_actor(actor)
{
}

ActorId::ActorId( const ActorId& id )
        : m_folk( id.m_folk ), m_group( id.m_group ), m_actor( id.m_actor )
{
}

ActorId::ActorId( const QString& actorid )
        : m_folk( Null )
{
    QUrl url( actorid );
    if ( !url.isValid() || url.scheme() != "actor" )
        return;
    if ( url.host() == "wavelet" )
        m_folk = Wavelet;
    else if ( url.host() == "federation" )
        m_folk = Federation;
    else if ( url.host() == "client" )
        m_folk = Client;
    else
        return;

    QString path = url.path();
    if ( path.isEmpty() || path[0] != '/' )
    {
        m_folk = Null;
        return;
    }

    int index = path.indexOf( '/', 1 );
    if ( index == -1 )
        m_group = path.mid(1);
    else
    {
        m_group = path.mid(1, index - 1);
        m_actor = path.mid( index + 1 );
    }
}

QString ActorId::toString() const
{
    QUrl url;
    url.setScheme("actor");
    switch( m_folk )
    {
        case Wavelet:
            url.setHost("wavelet");
            break;
        case Federation:
            url.setHost("federation");
            break;
        case Client:
            url.setHost("client");
            break;
        case Null:
        case MAX_FOLK:
            return QString::null;
    }

    QString path = "/" + m_group;
    if ( !m_actor.isEmpty() )
        path += "/" + m_actor;
    url.setPath( path );

    return url.toString();
}
