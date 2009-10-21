#ifndef WAVELET_H
#define WAVELET_H

#include <QObject>
#include <QHash>
#include <QList>

class Wave;
class Blip;
class BlipThread;
class StructuredDocument;
class Participant;
class Environment;

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
    QList<Blip*> rootBlips() const;
    Blip* blip(const QString& id);
    Environment* environment() const;

    void addParticipant( Participant* participant);
    void removeParticipant( Participant* participant);
    QList<Participant*> participants() const { return m_participants; }

    void updateConversation();

    void print_();

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
};

#endif // WAVELET_H
