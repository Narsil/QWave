#ifndef DIGEST_H
#define DIGEST_H

#include <QObject>

class OTProcessor;
class Environment;
class DocumentMutation;

class Digest : public QObject
{
    Q_OBJECT
public:
    Digest(Environment* environment, QObject* parent = 0);

    OTProcessor* processor() const { return m_processor; }

private slots:
    /**
      * Connected to the OTProcessor.
      */
    void addParticipant( const QString& address, const QString& waveid );
    /**
      * Connected to the OTProcessor.
      */
    void removeParticipant( const QString& address, const QString& waveid );
    /**
      * Connected to the OTProcessor.
      */
    void mutateDocument( const QString& documentId, const DocumentMutation& mutation, const QString& waveid );

private:    
    OTProcessor* m_processor;
    Environment* m_environment;
};

#endif // DIGEST_H
