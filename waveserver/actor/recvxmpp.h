#ifndef RecvXmpp_H
#define RecvXmpp_H

#include "waitingcondition.h"
#include "recvxor.h"
#include "network/xmppstanza.h"

template<XmppStanza::Kind _kind> class RecvXmppImpl : public WaitingConditionImpl
{
public:
    RecvXmppImpl() : m_message(0) { }
    RecvXmppImpl( const QString& stanzaId ) : m_message(0), m_stanzaId(stanzaId) { }

    inline XmppStanza* message() const { return m_message; }

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        if ( m_message )
            return this;
        XmppStanza* ptr = dynamic_cast<XmppStanza*>( event );
        if ( ptr && ptr->kind() == _kind )
        {
            if ( m_stanzaId.isNull() || m_stanzaId == ptr->stanzaId() )
            {
                m_message = ptr;
                return this;
            }
        }
        return 0;
    }

private:
    XmppStanza* m_message;
    QString m_stanzaId;
};

template<XmppStanza::Kind M> class RecvXmpp : public WaitingCondition
{
public:
    RecvXmpp() { m_ptr = new RecvXmppImpl<M>(); m_ptr->m_refCount = 1; }
    RecvXmpp(const QString& id) { m_ptr = new RecvXmppImpl<M>(id); m_ptr->m_refCount = 1; }
    RecvXmpp( const RecvXmpp<M>& x ) { m_ptr = const_cast<RecvXmppImpl<M>*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    RecvXmpp( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<RecvXmppImpl<M>*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    ~RecvXmpp() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    XmppStanza* operator->() const { return m_ptr->message(); }
    XmppStanza& operator*() const { return *(m_ptr->message()); }
    operator bool() const { return m_ptr != 0; }

    RecvXmpp<M>& operator=( const RecvXmpp<M>& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const RecvXmpp<M>& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const RecvXmpp<M>& x) const { return m_ptr != x.m_ptr; }

    RecvXmppImpl<M>* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    const XmppStanza& message() const { Q_ASSERT( m_ptr ); return *(m_ptr->message()); }

    typedef RecvXmppImpl<M> ImplType;

private:
    RecvXmppImpl<M>* m_ptr;
};

template<XmppStanza::Kind M, class X> RecvXor operator|( const RecvXmpp<M>& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

typedef RecvXmpp<XmppStanza::WaveletUpdate> RecvWaveletUpdate;

#endif // RecvXmpp_H
