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
    IMessage() : m_receiver(0) { }
    IMessage( const ActorId& sender ) : m_receiver(0), m_sender(sender) { }
    virtual ~IMessage() { }

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
};

#endif // IMESSAGE_H
