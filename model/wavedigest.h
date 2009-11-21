#ifndef WAVEDIGEST_H
#define WAVEDIGEST_H

#include <QObject>
#include <QList>
#include <QString>

class Wave;
class Participant;
class StructuredDocument;
class DocumentMutation;
class OTProcessor;

/**
  * Contains a digest of the wave. The contents of the digest is
  * calculated at the server.
  */
class WaveDigest : public QObject
{
    Q_OBJECT
public:
    WaveDigest(Wave* parent,OTProcessor* processor);

    Wave* wave() const;

    void addParticipant( Participant* participant);
    void removeParticipant( Participant* participant);
    QList<Participant*> participants() const { return m_participants; }
    Participant* participant( const QString& address );

    OTProcessor* processor() const { return m_processor; }

    QString toPlainText() const;

private slots:
    /**
      * Connected to the OTProcessor.
      */
    void addParticipant( const QString& address );
    /**
      * Connected to the OTProcessor.
      */
    void removeParticipant( const QString& address );
    /**
      * Connected to the OTProcessor.
      */
    void mutateDocument( const QString& documentId, const DocumentMutation& mutation, const QString& author );

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
    OTProcessor* m_processor;
};

#endif // WAVEDIGEST_H
