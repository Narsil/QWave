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
    PBMessage( const ActorId& receiver, qint64 id = -1) : IMessage( receiver, id ), PB() { }
    PBMessage( const PB& protoBuf, const ActorId& receiver, qint64 id = -1 ) : IMessage( receiver, id ), PB() { MergeFrom( protoBuf ); }
    PBMessage( const PBMessage<PB>& message ) : IMessage( message ), PB() { MergeFrom( message ); }
    PBMessage( const QByteArray& data, const ActorId& receiver, qint64 id = -1 ) : IMessage( receiver, id ), PB() { this->ParseFromArray( data.constData(), data.length() ); }

    QByteArray serialize() const { QByteArray ba( this->ByteSize(), 0 ); this->SerializeToArray( ba.data(), ba.length() ); return ba; }
};

#endif // PBMESSAGE_H
