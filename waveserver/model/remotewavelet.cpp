#include "remotewavelet.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "model/participant.h"

RemoteWavelet::RemoteWavelet(Wave* wave, const QString& waveletDomain, const QString& waveletId)
        : Wavelet( wave, waveletDomain, waveletId )
{
}

bool RemoteWavelet::apply( AppliedWaveletDelta& appliedDelta, QString* errorMessage )
{
    // Make a copy of the delta because we might have to transform it
    WaveletDelta clientDelta( appliedDelta.signedDelta().delta() );

    // Check its applicability
    if ( !checkHashedVersion( clientDelta, errorMessage ) )
        return false;

    // Transform if required
    bool ok;
    bool transformed = transform( clientDelta, errorMessage, &ok );
    if ( !ok )
        return false;
    if ( transformed )
        appliedDelta.setTransformedDelta( clientDelta );

    // TODO: Rollback if something went wrong, or report that only a subset of ops succeeded

    for( QList<WaveletDeltaOperation>::const_iterator it = clientDelta.operations().begin(); it != clientDelta.operations().end(); it++ )
    {
        QString docId = (*it).documentId();
        WaveletDocument* doc = m_documents[docId];
        if ( !doc )
        {
            doc = new WaveletDocument();
            m_documents[docId] = doc;
        }

        if ( (*it).hasMutation() )
        {
            bool check = doc->apply( (*it).mutation(), clientDelta.author() );
            if ( !check )
            {
                // TODO: rollback
                errorMessage->append("Failed to apply delta to " + docId);
                return false;
            }
            // Remember that the digest will need an update
        }
        if ( (*it).hasAddParticipant() )
        {
            QString p = (*it).addParticipant();
            JID jid(p);
            if ( !jid.isValid() )
            {
                errorMessage->append("Invalid JID " + p );
                return false;
            }
            if ( !m_participants.contains( p ) )
            {
                m_participants.insert( p );
                if ( jid.isLocal() )
                {
                    // Add the wavelet to the participant (and  make sure that such a participant exists.
                    // TODO: Error if we know that this participant is not known?
                    Participant::participant( p, true )->addWavelet(this);
                }
            }
        }
        if ( (*it).hasRemoveParticipant() )
        {
            QString p = (*it).removeParticipant();
            JID jid(p);
            if ( !jid.isValid() )
            {
                errorMessage->append("Invalid JID " + p );
                return false;
            }
            if ( m_participants.contains( p ) )
            {
                m_participants.remove( p );
                if ( jid.isLocal() )
                {
                    // Remove the wavelet from the participant
                    Participant* pptr = Participant::participant( p, false );
                    if ( pptr )
                        pptr->removeWavelet(this);
                }
            }
        }
    }

    // Send the delta to all local subscribers
    commit( appliedDelta );

    return true;
}

bool RemoteWavelet::isRemote() const
{
    return true;
}

bool RemoteWavelet::isLocal() const
{
    return false;
}
