#ifndef WAVELET_H
#define WAVELET_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QUrl>

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

    QString id() const { return this->m_id; }
    QString domain() const { return this->m_domain; }
    Wave* wave() const { return this->m_wave; }
    StructuredDocument* document() const { return m_doc; }
    QList<Blip*> rootBlips() const { return m_rootBlips; }
    Blip* blip(const QString& id);
    Environment* environment() const;
    OTProcessor* processor() const { return m_processor; }
    QUrl url() const;
    Attachment* attachment( const QString& id ) const;

    void addParticipant( Participant* participant);
    void removeParticipant( Participant* participant);
    QList<Participant*> participants() const { return m_participants; }
    Participant* participant( const QString& address );

    int blipCount() const;
    int unreadBlipCount() const;
    void setUnread(bool unread);

    void print_();

    // TODO: This should become private
    void updateConversation(const QString& author);

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
      * Consumed by the GUI.
      */
    void participantAdded(Participant* participant);
    /**
      * Consumed by the GUI.
      */
    void participantRemoved(Participant* participant);
    /**
      * Consumed by the GUI.
      */
    void conversationChanged();
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
    QString m_id;
    QString m_domain;
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
    QHash<QString,Blip*> m_blips;
    QHash<QString,BlipThread*> m_blipThreads;
    QList<Participant*> m_participants;
    OTProcessor* m_processor;
    QHash<QString, UnknownDocument*> m_unknownDocs;
    QHash<QString, Attachment*> m_attachments;
    QList<Blip*> m_rootBlips;
};

#endif // WAVELET_H
