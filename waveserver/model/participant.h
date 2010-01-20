#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QString>
#include <QSet>

class Wavelet;

class Participant
{
public:
    Participant(const QString& id);
    ~Participant();

    QString id() const { return m_id; }

    void addWavelet( Wavelet* wavelet );
    void removeWavelet( Wavelet* wavelet );
    QSet<Wavelet*> wavelets() const;

    static Participant* participant( const QString& id, bool create = false );

private:
    QString m_id;
    /**
      * List of all 'conv+root' wavelets in which this user participates.
      */
    QSet<Wavelet*> m_wavelets;

    static QHash<QString,Participant*>* s_participants;
};

#endif // PARTICIPANT_H
