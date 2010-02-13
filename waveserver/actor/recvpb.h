#ifndef RECVPB_H
#define RECVPB_H

#include "actor/waitingcondition.h"
#include "actor/recvxor.h"
#include "actor/pbmessage.h"

template<class PB> class RecvPBImpl : public WaitingConditionImpl
{
public:
    RecvPBImpl() : m_id(-1) { }
    RecvPBImpl( qint64 id ) : m_id(id) { }

    QSharedPointer<PBMessage<PB> > message() const { return m_message; }

    virtual WaitingConditionImpl* handleMessage( const QSharedPointer<IMessage>& msg )
    {
        if ( m_message )
            return this;
        PBMessage<PB>* ptr = dynamic_cast<PBMessage<PB>*>( msg.data() );
        if ( ptr )
        {
            if ( m_id == -1 || m_id == ptr->id() )
            {
                m_message = msg.dynamicCast<PBMessage<PB> >();
                return this;
            }
        }
        return 0;
    }

    QSharedPointer<PBMessage<PB> > m_message;
    qint64 m_id;
};

template<class PB> class RecvPB : public WaitingCondition
{
public:
    RecvPB() { m_ptr = new RecvPBImpl<PB>(); m_ptr->m_refCount = 1; }
    RecvPB( qint64 id) { m_ptr = new RecvPBImpl<PB>(id); m_ptr->m_refCount = 1; }
    RecvPB( const RecvPB<PB>& x ) { m_ptr = const_cast<RecvPBImpl<PB>*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    RecvPB( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<RecvPBImpl<PB>*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    ~RecvPB() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    PB* operator->() const { return &m_ptr->m_message.data()->protoBuf(); }
    PB& operator*() const { return m_ptr->m_message.data()->protoBuf(); }
    operator bool() const { return m_ptr != 0; }

    RecvPB<PB>& operator=( const RecvPB<PB>& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const RecvPB<PB>& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const RecvPB<PB>& x) const { return m_ptr != x.m_ptr; }

    const PBMessage<PB>& message() const { return *(m_ptr->m_message.data()); }

    RecvPBImpl<PB>* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    typedef RecvPBImpl<PB> ImplType;

private:
    RecvPBImpl<PB>* m_ptr;
};

template<class PB, class X> RecvXor operator|( const RecvPB<PB>& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

#endif // RECVPB_H
