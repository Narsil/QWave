#ifndef PBMESSAGE_H
#define PBMESSAGE_H

#include <QByteArray>

#include "actor/imessage.h"

template<class PB> class PBMessage : public IMessage
{
public:
    PBMessage() { }
    PBMessage( const PB& protoBuf, qint64 id = 0 ) : m_id(id) { m_data.MergeFrom( protoBuf ); }
    PBMessage( const PBMessage<PB>& message ) : IMessage( message ), m_id( message.m_id ) { m_data.MergeFrom( message.m_data ); }
    PBMessage( const QByteArray& data ) { m_data.ParseFromArray( data.constData(), data.length() ); }

    QByteArray serialize() const { QByteArray ba( m_data.ByteSize(), 0 ); m_data.SerializeToArray( ba.constData(), ba.length() ); return ba; }

    qint64 id() const { return m_id; }
    void setId( const qint64& id ) { m_id = id; }

    const PB& protoBuf() const { return m_data; }
    PB& protoBuf() { return m_data; }

    void PrintDebugString() const { m_data.PrintDebugString(); }

    PB* operator->() { return &m_data; }
    const PB* operator->() const { return &m_data; }

    PB& operator*() { return m_data; }
    const PB& operator*() const { return m_data; }

private:
    PB m_data;
    qint64 m_id;
};

#endif // PBMESSAGE_H
