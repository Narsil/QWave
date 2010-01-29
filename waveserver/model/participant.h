#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>
#include <QSet>
#include "model/jid.h"

class Wavelet;

class Participant
{
public:
    // Participant(const QString& jid);
    Participant(const JID& jid);
    ~Participant();

    JID jid() const { return m_jid; }
    /**
      * @returns the JID as string.
      */
    QString toString() const { return m_jid.toString(); }
    bool isLocal() const { return m_jid.isLocal(); }

    void addWavelet( Wavelet* wavelet );
    void removeWavelet( Wavelet* wavelet );
    QSet<Wavelet*> wavelets() const;

    static Participant* participant( const QString& jid, bool create = false );

private:
    JID m_jid;
    /**
      * List of all 'conv+root' wavelets in which this user participates.
      */
    QSet<Wavelet*> m_wavelets;

    static QHash<QString,Participant*>* s_participants;
};

#endif // PARTICIPANT_H
