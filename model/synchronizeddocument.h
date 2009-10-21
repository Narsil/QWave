#ifndef SYNCHRONIZEDDOCUMENT_H
#define SYNCHRONIZEDDOCUMENT_H

#include <QList>

#include "documentmutation.h"
#include "structureddocument.h"

class SynchronizedDocument : public StructuredDocument
{
    Q_OBJECT
public:
    SynchronizedDocument(QObject* parent = 0);

    void handleSend( DocumentMutation& incoming );
    void handleReceive( const DocumentMutation& incoming );

private:
    int m_serverMsgCount;
    int m_clientMsgCount;
    QList<DocumentMutation> m_outgoingMutations;
};

#endif // SYNCHRONIZEDDOCUMENT_H
