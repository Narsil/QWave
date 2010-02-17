#ifndef WAVELET_H
#define WAVELET_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QUrl>
#include "model/waveurl.h"

class Wave;
class Blip;
class BlipThread;
class StructuredDocument;
class Participant;
class Environment;
class OTProcessor;
class DocumentMutation;
class UnknownDocument;
class Attachment;
class QImage;

class Wavelet : public QObject
{
    Q_OBJECT
public:
    Wavelet(Wave* wave, const QString& domain, const QString &id);
    ~Wavelet();

    /**
      * @return Id of the wavelet.
      */
    QString id() const { return this->m_id; }
    /**
      * @return the domain of the wavelet.
      */
    QString domain() const { return this->m_domain; }
    Wave* wave() const { return this->m_wave; }
    /**
      * @return the conversation document which represents the structure of blips and threads.
      */
    StructuredDocument* document() const { return m_doc; }
    QList<Blip*> rootBlips() const { return m_rootBlips; }
    Blip* rootBlip( const QString& id ) const;
    Blip* blip(const QString& id) const;
    Environment* environment() const;
    OTProcessor* processor() const { return m_processor; }
    WaveUrl url() const;
    Attachment* attachment( const QString& id ) const;

    void addParticipant( Participant* participant);
    void removeParticipant( Participant* participant);
    QList<Participant*> participants() const { return m_participants; }
    Participant* participant( const QString& address );

    int blipCount() const;
    int unreadBlipCount() const;
    void setUnread(bool unread);

    Blip* createRootBlip(const QString& text = QString::null);

    void print_();

    /**
      * Adds an attachment document to the wavelet.
      *
      * @return the generated attachment ID.
      *
      * Note that in addition you may want to add the image to a blip.
      */
    QString insertImageAttachment(const QUrl& url, const QImage& image, const QImage& thumbnail);

signals:
    /**
      * Consumed by the Robot
      */
    void participantAdded(Participant* participant, Wavelet* wavelet);
    /**
      * Consumed by the Robot
      */
    void participantRemoved(Participant* participant, Wavelet* wavelet);
    /**
      * Consumed by the Robot
      */
    void conversationChanged(Wavelet* wavelet);
    /**
      * Consumed but the Robot
      */
    void newBlipSubmitted(Blip* blip);
    /**
      * Emitted when the number of blips or the number of read/unread blip changed.
      *
      * Consumed by the wave.
      */
    void blipCountChanged();

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

private:
    /**
      * Called from mutateDocument() when the conversation document changes.
      */
    void updateConversation(const QString& author);

    /**
      * Id of the wavelet.
      */
    QString m_id;
    /**
      * Domain of the wavelet.
      */
    QString m_domain;
    /**
      * The wave to which this wavelet belongs.
      */
    Wave* m_wave;
    /**
      * Stores the structure of the conversation. This document can look like:
      * <conversation sort="m">
      *  <blip id="b+a">
      *    <thread inline="false" id="r1">
      *      <blip id="b+b"/>
      *      <blip id="b+c"/>
      *    </thread>
      *  </blip>
      * </conversation>
      *
      * @see http://www.waveprotocol.org/draft-protocol-specs/wave-conversation-model
      */
    StructuredDocument* m_doc;
    /**
      * Map of all blips in the conversation.
      */
    QHash<QString,Blip*> m_blips;
    /**
      * Map of all blip threads in the conversation.
      */
    QHash<QString,BlipThread*> m_blipThreads;
    /**
      * All participants belonging to this wavelet.
      */
    QList<Participant*> m_participants;
    /**
      * OTProcessor which handles OT and corresponds with the wave server.
      */
    OTProcessor* m_processor;
    /**
      * Map of wavelet documents which are of an unknown kind, i.e. neither blip, nor conversation or attachment.
      * The keys are document names.
      */
    QHash<QString, UnknownDocument*> m_unknownDocs;
    /**
      * Map of all attachment documents belonging to the wavelet.
      * The keys are document names.
      */
    QHash<QString, Attachment*> m_attachments;
    /**
      * List of all root blips.
      */
    QList<Blip*> m_rootBlips;
};

#endif // WAVELET_H
