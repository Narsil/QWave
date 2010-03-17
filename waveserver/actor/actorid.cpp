#include "actorid.h"
#include "actorgroup.h"
#include "actorfolk.h"
#include <QUrl>

ActorId::ActorId( ActorGroup* group, const QString& actor )
        : m_folk( group->folk()->folkId() ), m_group( group->absGroupId() ), m_actor( actor )
{
}

ActorId::ActorId(const QString& folk, const QString& group, const QString& actor)
        : m_folk(folk), m_group(group), m_actor(actor)
{
}

ActorId::ActorId( const ActorId& id )
        : m_folk( id.m_folk ), m_group( id.m_group ), m_actor( id.m_actor )
{
}

ActorId::ActorId( const QString& actorid )
{
    QUrl url( actorid );
    if ( !url.isValid() || url.scheme() != "actor" )
        return;
    m_folk = url.host();

    QString path = url.path();
    if ( path.isEmpty() || path[0] != '/' )
    {
        m_folk = QString::null;
        return;
    }

    int index = path.lastIndexOf( '/' );
    if ( index == -1 || index == 0 )
    {
        m_folk = QString::null;
        return;
    }

    if ( index == path.length() - 1 )
        m_group = path.mid(1, path.length() - 2);
    else
    {
        m_group = path.mid(1, index - 1);
        m_actor = path.mid( index + 1 );
    }
}

ActorId& ActorId::operator=( const ActorId& id )
{
    m_folk = id.folk();
    m_group = id.group();
    m_actor = id.actor();
    return *this;
}

QString ActorId::toString() const
{
    if ( isNull() )
        return QString::null;

    QUrl url;
    url.setScheme("actor");
    url.setHost( m_folk );

    QString path = "/" + m_group;
    if ( !m_actor.isEmpty() )
        path += "/" + m_actor;
    else
        path += "/";
    url.setPath( path );

    return url.toString();
}

bool operator==( const ActorId& id1, const ActorId& id2 )
{
    return ( id1.folk() == id2.folk() && id1.group() == id2.group() && id1.actor() == id2.actor() );
}
