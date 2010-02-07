#ifndef IMESSAGE_H
#define IMESSAGE_H

class Actor;

class IMessage
{
public:
    IMessage() : m_actor(0) { }
    virtual ~IMessage() { }

    void setReceiver( Actor* actor ) { m_actor = actor; }
    Actor* receiver() const { return m_actor; }

private:
    Actor* m_actor;
};

#endif // IMESSAGE_H
