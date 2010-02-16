#ifndef IMESSAGE_H
#define IMESSAGE_H

#include "actor/actorid.h"

class Actor;

/**
  * All messages which are passed between actors must implement this interface.
  */
class IMessage
{
public:
    IMessage() : m_receiver(0), m_createOnDemand(false) { }
    IMessage( const ActorId& sender ) : m_receiver(0), m_sender(sender), m_createOnDemand(false) { }
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
    void setReceiver( Actor* receiver ) { m_receiver = receiver; }
    /**
      * @internal
      */
    Actor* receiver() const { return m_receiver; }

protected:
    IMessage( const IMessage& msg ) : m_receiver( msg.m_receiver ), m_sender( msg.m_sender ) { }\

private:
    Actor* m_receiver;
    ActorId m_sender;
    bool m_createOnDemand;
};

#endif // IMESSAGE_H
