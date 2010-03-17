#ifndef ACTORID_H
#define ACTORID_H

#include <QString>
#include <QStringList>

class ActorGroup;

/**
  * Uniquely identifies an actor or an actor group.
  * Such an ID can be used to send a message using a symbolic name, i.e. the ActorId.
  *
  * An actor ID has one of the following forms:
  * actor://<Folk>/<Group>/
  * actor://<Folk>/<Group>/<Actor>
  *
  * Here <Folk> is the string "wavelet", "federation", "client", or "store".
  * The <Group> is any valid URL path. It can contain slashes. In this case it is called a hierarchical group.
  *
  * The first adressing scheme broadcasts a message to all actors in a group.
  * The second adressing scheme sends a message to a certain actor in a group.
  */
class ActorId
{
public:
    ActorId() { }
    ActorId( ActorGroup* group, const QString& actor );
    ActorId( const QString& folk, const QString& group, const QString& actor = QString::null);
    ActorId( const ActorId& id );
    ActorId( const QString& actorid );

    inline QString folk() const { return m_folk; }
    /**
      * @return the group name which can contain slashes if it is a hierarchical group.
      */
    inline QString group() const { return m_group; }
    /**
      * @return the list of group names in a hierarchical group.
      */
    inline QStringList groups() const { return m_group.split('/'); }
    inline QString actor() const { return m_actor; }

    inline bool hasActor() const { return !m_actor.isEmpty(); }

    ActorId& operator=( const ActorId& id );

    QString toString() const;

    inline bool isNull() const { return m_folk.isNull() || m_group.isNull(); }

private:
    QString m_folk;
    QString m_group;
    QString m_actor;
};

bool operator==( const ActorId& id1, const ActorId& id2 );

#endif // ACTORID_H
