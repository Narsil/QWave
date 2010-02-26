#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>
#include <QSet>
#include "model/jid.h"
#include "actor/actorgroup.h"
#include "actor/actor.h"
#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"

class ParticipantFolk;

class Participant : public ActorGroup
{
public:
    Participant(const JID& jid, ParticipantFolk* folk);
    ~Participant();

    JID jid() const { return m_jid; }
    /**
      * @returns the JID as string.
      */
    QString toString() const { return m_jid.toString(); }
    bool isLocal() const { return m_jid.isLocal(); }

    QSet<QString> wavelets() const;

//protected:
//    virtual void customEvent( QEvent* event );

private:
    void addWavelet( const QString& wavelet );
    void removeWavelet( const QString& wavelet );

    /**
      * Abstract base class for actors working on participants.
      */
//    class ParticipantActor : public Actor
//    {
//    public:
//        ParticipantActor( Participant* p ) : Actor( QString::number( s_id++ ), p ), m_participant(p) { }
//
//    protected:
//        Participant* m_participant;
//
//    private:
//        static qint64 s_id;
//    };
//
//    class AddRemoveWaveletActor : public ParticipantActor
//    {
//    public:
//        AddRemoveWaveletActor( Participant* p, PBMessage<messages::ParticipantAddRemoveWavelet>* message );
//
//    protected:
//        virtual void execute();
//
//    private:
//        PBMessage<messages::ParticipantAddRemoveWavelet> m_message;
//    };
//
//    class QueryWaveletsActor : public ParticipantActor
//    {
//    public:
//        QueryWaveletsActor( Participant* p, PBMessage<messages::ParticipantQueryWavelets>* message );
//
//    protected:
//        virtual void execute();
//
//    private:
//        PBMessage<messages::ParticipantQueryWavelets> m_message;
//    };

    JID m_jid;
    /**
      * List of all 'conv+root' wavelets in which this user participates.
      */
    QSet<QString> m_wavelets;
};

#endif // PARTICIPANT_H
