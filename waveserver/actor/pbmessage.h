#ifndef PBMESSAGE_H
#define PBMESSAGE_H

#include <QByteArray>

#include "actor/imessage.h"

/**
  * A message containing a protocol buffer message.
  */
template<class PB> class PBMessage : public IMessage, public PB
{
public:
    PBMessage() { }
    PBMessage(qint64 id) : m_id(id) { }
    PBMessage( const PB& protoBuf, qint64 id = 0 ) : m_id(id) { MergeFrom( protoBuf ); }
    PBMessage( const PBMessage<PB>& message ) : IMessage( message ), m_id( message.m_id ) { MergeFrom( message ); }
    PBMessage( const QByteArray& data ) { this->ParseFromArray( data.constData(), data.length() ); }

    QByteArray serialize() const { QByteArray ba( this->ByteSize(), 0 ); this->SerializeToArray( ba.constData(), ba.length() ); return ba; }

    qint64 Id() const { return m_id; }
    void SetId( const qint64& id ) { m_id = id; }

private:
    qint64 m_id;
};

#endif // PBMESSAGE_H
