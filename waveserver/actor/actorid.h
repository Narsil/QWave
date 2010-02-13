#ifndef ACTORID_H
#define ACTORID_H

#include <QString>

/**
  * Uniquely identifies an actor or an actor group.
  * Such an ID can be used to send a message using a symbolic name, i.e. the ActorId.
  */
class ActorId
{
public:
    enum Folk
    {
        Null = 0,
        Wavelet = 1,
        Federation = 2,
        Client = 3,
        MAX_FOLK = 4
    };

    ActorId() : m_folk(Null) { }
    ActorId(Folk folk, const QString& group, const QString& actor = QString::null);
    ActorId( const ActorId& id );
    ActorId( const QString& actorid );

    inline Folk folk() const { return m_folk; }
    inline QString group() const { return m_group; }
    inline QString actor() const { return m_actor; }

    inline bool hasActor() const { return !m_actor.isEmpty(); }

    QString toString() const;

    inline bool isNull() const { return m_folk == Null; }

private:
    Folk m_folk;
    QString m_group;
    QString m_actor;
};

#endif // ACTORID_H
