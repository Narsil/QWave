#include "actorgroup.h"
#include "actor.h"
#include "actorfolk.h"
#include "actordispatcher.h"
#include <QCoreApplication>

ActorGroup::ActorGroup(const QString& groupId, ActorFolk* folk)
        : QObject( folk )
{
    setObjectName( groupId );
}

ActorGroup::ActorGroup(const QString& groupId, ActorGroup* parentGroup)
        : QObject( parentGroup )
{
    setObjectName( groupId );
}

ActorGroup::~ActorGroup()
{
}

Actor* ActorGroup::actor( const QString& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    return findDirectChild<Actor>( id );
}

ActorGroup* ActorGroup::group( const QString& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    return findDirectChild<ActorGroup>( id );
}

QString ActorGroup::absGroupId() const
{
    ActorGroup* p = parentGroup();
    if ( p )
    {
        QString id = p->groupId();
        id += "/";
        id += objectName();
        return id;
    }
    return objectName();
}

ActorId ActorGroup::actorId() const
{
    ActorFolk* f = folk();
    if ( !f )
        return ActorId();
    return ActorId( f->folkId(), absGroupId() );
}

ActorFolk* ActorGroup::folk() const
{
    ActorFolk* f = dynamic_cast<ActorFolk*>( parent() );
    if ( f )
        return f;
    ActorGroup* p = parentGroup();
    if ( p )
        return p->folk();
    return 0;
}

template<class T> T* ActorGroup::findDirectChild( const QString& name )
{
    for( int i = 0; i < children().length(); ++i )
    {
        QObject* c = children().at(i);
        if ( c->objectName() == name )
            return dynamic_cast<T*>( c );
    }
    return 0;
}

void ActorGroup::customEvent( QEvent* event )
{
    for( int i = 0; i < children().length(); ++i )
    {
        Actor* a = dynamic_cast<Actor*>( children().at(i) );
        if ( a )
            a->event( event );
    }
}

bool ActorGroup::post( IMessage* msg )
{
    if ( msg->sender().isNull() )
        msg->setSender( actorId() );
    return ActorDispatcher::dispatcher()->post( msg );
}
