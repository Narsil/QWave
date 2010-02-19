#ifndef IMESSAGE_H
#define IMESSAGE_H

#include <QEvent>

#include "actor/actorid.h"

/**
  * All messages which are passed between actors must implement this interface.
  */
class IMessage : public QEvent
{
public:
    enum EventType
    {
        Message = QEvent::User,
        Create = QEvent::User + 1
    };

    IMessage() : QEvent( (QEvent::Type)Message ), m_id(-1), m_createOnDemand(false) { }
    IMessage(qint64 id) : QEvent( (QEvent::Type)Message ), m_id(id), m_createOnDemand(false) { }
    IMessage( const ActorId& receiver ) : QEvent( (QEvent::Type)Message ), m_id(-1), m_receiver(receiver), m_createOnDemand(false) { }
    IMessage( const ActorId& receiver, qint64 id ) : QEvent( (QEvent::Type)Message ), m_id(id), m_receiver(receiver), m_createOnDemand(false) { }
    virtual ~IMessage() { }

    /**
      * If this flag is set (by default it is false), the receiving actor is created
      * if it does not exist yet. However, creating can still fail, but at least it is attempted.
      * If this flag is false and the actor is neither instantiated nor persisted
      * (i.e. the actor is neither in RAM nor on disk) when the message arrives,
      * the message will be discarded.
      */
    void setCreateOnDemand( bool enable ) { m_createOnDemand = enable; }
    bool createOnDemand() const { return m_createOnDemand; }

    qint64 id() const { return m_id; }
    void setId( const qint64& id ) { m_id = id; }

    /**
      * Setting a sender allows the recepient of the message to reply.
      */
    void setSender( const ActorId& sender ) { m_sender = sender; }
    const ActorId& sender() const { return m_sender; }

    /**
      * @internal
      *
      * During dispatch the receiving actor is resolved and stored here for convenience.
      * Setting the receiver when sending a message has no effect or could even result in errors.
      * This is only for internal caching use.
      */
    void setReceiver( const ActorId& receiver ) { m_receiver = receiver; }
    /**
      * @internal
      */
    const ActorId& receiver() const { return m_receiver; }

protected:
    IMessage( const IMessage& msg ) : QEvent( msg.type() ), m_id( msg.m_id ), m_sender( msg.m_sender), m_receiver( msg.m_receiver ), m_createOnDemand( msg.m_createOnDemand ) { }\

private:
    qint64 m_id;
    ActorId m_sender;
    ActorId m_receiver;
    bool m_createOnDemand;
};

#endif // IMESSAGE_H
