#ifndef STORE_H
#define STORE_H

#include <QHash>
#include <QByteArray>
#include <QSet>
#include <string>

#include "actor/actorgroup.h"
#include "protocol/messages.pb.h"

class StoreFolk;
class CommitLog;

class Store : public ActorGroup
{
public:
    Store(const QString& id, StoreFolk* folk);
    ~Store();

protected:
    void customEvent( QEvent* event );

    class WaveletUpdate : public messages::StoreWaveletUpdate
    {
    public:
        WaveletUpdate() : m_prev(0), m_next(0) {  }

        void setPrev( WaveletUpdate* prev ) { m_prev = prev; }
        WaveletUpdate* prev() const { return m_prev; }
        void setNext( WaveletUpdate* next ) { m_next = next; }
        WaveletUpdate* next() const { return m_next; }

    private:
        WaveletUpdate* m_prev;
        WaveletUpdate* m_next;
    };

private:
    class Participant
    {
    public:
        Participant( const QString& jid ) : m_jid( jid ) { }

        QSet<QByteArray> m_wavelets;
        QString m_jid;
    };

    bool writeToMemory( const messages::PersistWaveletUpdate& update );
    Participant* participant( const std::string& name );

    CommitLog* m_commitLog;
    QHash<QByteArray,WaveletUpdate*> m_wavelets;
    QHash<QByteArray,Participant*> m_participants;
};

#endif // STORE_H
