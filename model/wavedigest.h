#ifndef WAVEDIGEST_H
#define WAVEDIGEST_H

#include <QObject>
#include <QList>
#include <QString>

class Wave;
class Participant;
class StructuredDocument;
class DocumentMutation;

class WaveDigest : public QObject
{
    Q_OBJECT
public:
    WaveDigest(Wave* parent);

    Wave* wave() const;

    void addParticipant( Participant* participant);
    void removeParticipant( Participant* participant);
    QList<Participant*> participants() const { return m_participants; }
    Participant* participant( const QString& address );

    QString toPlainText() const;

    void mutate(const DocumentMutation& mutation);

signals:
    /**
      * Consumed by the GUI.
      */
    void digestChanged();
    /**
      * Consumed by the GUI.
      */
    void participantAdded(Participant* participant);
    /**
      * Consumed by the GUI.
      */
    void participantRemoved(Participant* participant);

private:
    QList<Participant*> m_participants;
    QString m_digest;
    StructuredDocument* m_doc;
};

#endif // WAVEDIGEST_H
