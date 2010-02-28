#include "store.h"
#include "storefolk.h"
#include "persistence/commitlog.h"
#include "actor/pbmessage.h"

Store::Store(const QString& id, StoreFolk* folk)
        : ActorGroup( id, folk )
{
    m_commitLog = new CommitLog( this );

    while( true )
    {
        QByteArray ba = m_commitLog->read();
        if ( ba.isNull() )
            break;

        messages::PersistWaveletUpdate update;
        update.ParseFromArray( ba.data(), ba.length() );

        if ( !writeToMemory( update ) )
        {
            qDebug("Failed to apply commit log");
            break;
        }
    }
}

Store::~Store()
{
    foreach( WaveletUpdate* update, m_wavelets.values() )
    {
        WaveletUpdate* p = update->prev();
        do
        {
            delete update;
            update = p;
            if ( update )
                p = update->prev();
        } while( update );
    }
    foreach( Participant* p, m_participants )
    {
        delete p;
    }
}

void Store::customEvent( QEvent* event )
{
    PBMessage<messages::PersistWaveletUpdate>* waveletUpdate = dynamic_cast<PBMessage<messages::PersistWaveletUpdate>*>( event );
    if ( waveletUpdate )
    {
        if ( !writeToMemory( *waveletUpdate ) )
        {
            qDebug("Could not write to memory");
            // TODO: Error handling
        }
        if ( !m_commitLog->write( waveletUpdate->serialize() ) )
        {
            qDebug("Could not write to commit log");
            // TODO: Error handling
        }

        // Tell the sender that the update has been accepted and committed
        PBMessage<messages::PersistAck>* ack = new PBMessage<messages::PersistAck>( waveletUpdate->sender(), waveletUpdate->id() );
        ack->set_ok( true );
        post( ack );
        return;
    }
    ////////////////////////////
    PBMessage<messages::QueryWaveletUpdates>* queryWaveletUpdate = dynamic_cast<PBMessage<messages::QueryWaveletUpdates>*>( event );
    if ( queryWaveletUpdate )
    {
        QByteArray ba = QByteArray::fromRawData( queryWaveletUpdate->wavelet_name().data(), queryWaveletUpdate->wavelet_name().length() );
        WaveletUpdate* w = m_wavelets[ ba ];
        // The wavelet is empty? Do report zero wavelet updates
        if ( !w )
        {
            PBMessage<messages::QueryWaveletUpdatesResponse>* response = new PBMessage<messages::QueryWaveletUpdatesResponse>( queryWaveletUpdate->sender(), queryWaveletUpdate->id() );
            response->set_ok(true);
            response->set_start_version( queryWaveletUpdate->start_version() );
            response->set_end_version( queryWaveletUpdate->start_version() );
            post( response );
        }

        WaveletUpdate* root = w;
        WaveletUpdate* last = 0;
        WaveletUpdate* first = 0;
        while( w )
        {
            if ( w->applied_at_version() >= queryWaveletUpdate->end_version() )
                last = w;
            if ( w->applied_at_version() < queryWaveletUpdate->start_version() )
                break;
            first = w;
            w = w->prev();
        }

        PBMessage<messages::QueryWaveletUpdatesResponse>* response = new PBMessage<messages::QueryWaveletUpdatesResponse>( queryWaveletUpdate->sender(), queryWaveletUpdate->id() );
        response->set_ok(true);
        if ( first == 0 || first == last )
        {
            response->set_start_version( queryWaveletUpdate->start_version() );
            response->set_end_version( queryWaveletUpdate->start_version() );
        }
        else
        {
            w = first;
            while( w && w != last )
            {
                response->add_applied_delta( w->applied_delta() );
                w = w->next();
            }
            response->set_start_version( first->applied_at_version() );
            if ( last == 0 )
                response->set_end_version( root->applied_at_version() + root->operations_applied() );
            else
                response->set_end_version( last->applied_at_version() );
        }
        post( response );
        return;
    }
    ///////////////////////////////
    PBMessage<messages::QueryParticipantWavelets>* queryParticipantWavelets = dynamic_cast<PBMessage<messages::QueryParticipantWavelets>*>( event );
    if ( queryParticipantWavelets )
    {
        Participant* p = participant( queryParticipantWavelets->participant() );
        PBMessage<messages::QueryParticipantWaveletsResponse>* response = new PBMessage<messages::QueryParticipantWaveletsResponse>( queryParticipantWavelets->sender(), queryParticipantWavelets->id() );
        response->set_ok( true );
        foreach( QString str, p->m_wavelets )
        {
            response->add_wavelet_name( str.toStdString() );
        }
        post( response );
        return;
    }

    this->ActorGroup::customEvent(event);
}

bool Store::writeToMemory( const messages::PersistWaveletUpdate& update )
{
    WaveletUpdate* w = new WaveletUpdate();
    QByteArray name( update.wavelet_name().data(), update.wavelet_name().length() );
    WaveletUpdate* p = m_wavelets[name];    
    // Is this indeed the next update?
    if ( p && p->applied_at_version() + p->operations_applied() != update.applied_at_version() )
    {
        qDebug("Version mismatch when applying update.");
        return false;
    }
    // Sanity check
    if ( update.applies_to_version() > update.applied_at_version() )
    {
        qDebug("Version mismatch when applying update.");
        return false;
    }
    w->set_applied_delta( update.applied_delta() );
    w->set_operations_applied( update.operations_applied() );
    w->set_applied_at_version( update.applied_at_version() );
    w->set_applies_to_version( update.applies_to_version() );
    w->setPrev( p );
    if ( p )
        p->setNext(w);
    m_wavelets[ name ] = w;
    for( int i = 0; i < update.add_user_size(); ++i )
    {
        Participant* p = participant( update.add_user(i) );
        p->m_wavelets.insert( name );
    }
    for( int i = 0; i < update.remove_user_size(); ++i )
    {
        Participant* p = participant( update.remove_user(i) );
        p->m_wavelets.remove( name );
    }

    return true;
}

Store::Participant* Store::participant( const std::string& name )
{
    QByteArray ba( name.data(), name.length() );
    Participant* result = m_participants[ba];
    if ( !result )
    {
        result = new Participant( ba );
        m_participants[ba] = result;
    }
    return result;
}
