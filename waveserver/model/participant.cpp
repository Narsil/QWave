#include "participant.h"
#include "model/participantfolk.h"
#include "actor/actorfolk.h"
#include "model/waveurl.h"

Participant::Participant(const JID& jid, ParticipantFolk* folk)
        : ActorGroup( jid.toString(), folk ), m_jid(jid)
{
}

Participant::~Participant()
{
}

void Participant::addWavelet( const QString& wavelet )
{
    m_wavelets.insert(wavelet);
}

void Participant::removeWavelet( const QString& wavelet )
{
    m_wavelets.remove(wavelet);
}

QSet<QString> Participant::wavelets() const
{
    return m_wavelets;
}

//void Participant::customEvent( QEvent* event )
//{
//    PBMessage<messages::ParticipantAddRemoveWavelet>* msg = dynamic_cast<PBMessage<messages::ParticipantAddRemoveWavelet>*>( event );
//    if ( msg )
//    {
//        new AddRemoveWaveletActor( this, msg );
//        return;
//    }
//    PBMessage<messages::ParticipantQueryWavelets>* query = dynamic_cast<PBMessage<messages::ParticipantQueryWavelets>*>( event );
//    if ( query )
//    {
//        new QueryWaveletsActor( this, query );
//        return;
//    }
//
//    this->ActorGroup::customEvent( event );
//}

/******************************************************
  *
  * ParticipantActor
  *
  *****************************************************/

//qint64 Participant::ParticipantActor::s_id = 0;

/******************************************************
  *
  * AddRemoveWaveletActor
  *
  *****************************************************/

//Participant::AddRemoveWaveletActor::AddRemoveWaveletActor( Participant* p, PBMessage<messages::ParticipantAddRemoveWavelet>* message )
//        : ParticipantActor( p ), m_message( *message )
//{
//}
//
//void Participant::AddRemoveWaveletActor::execute()
//{
//    BEGIN_EXECUTE;
//
//    WaveUrl url( QString::fromStdString( m_message.wavelet_name() ) );
//    if ( url.isNull() )
//    {
//        if ( m_message.id() != -1 )
//        {
//            PBMessage<messages::ParticipantAck>* ack = new PBMessage<messages::ParticipantAck>( m_message.sender(), m_message.id() );
//            ack->set_ok( false );
//            ack->set_error("Malformed Wavelet URL");
//            post( ack );
//        }
//        return;
//    }
//
//    if ( m_message.add() )
//        m_participant->addWavelet( url.toString() );
//    else
//        m_participant->removeWavelet( url.toString() );
//
//    if ( m_message.id() != -1 )
//    {
//        PBMessage<messages::ParticipantAck>* ack = new PBMessage<messages::ParticipantAck>( m_message.sender(), m_message.id() );
//        ack->set_ok( true );
//        post( ack );
//    }
//
//    END_EXECUTE;
//}

/******************************************************
  *
  * QueryWaveletsActor
  *
  *****************************************************/

//Participant::QueryWaveletsActor::QueryWaveletsActor( Participant* p, PBMessage<messages::ParticipantQueryWavelets>* message )
//        : ParticipantActor( p ), m_message( *message )
//{
//}
//
//void Participant::QueryWaveletsActor::execute()
//{
//    BEGIN_EXECUTE;
//
//    PBMessage<messages::ParticipantQueryWaveletsResponse>* response = new PBMessage<messages::ParticipantQueryWaveletsResponse>( m_message.sender(), m_message.id() );
//    response->set_ok( true );
//    foreach( QString str, m_participant->m_wavelets )
//    {
//        response->add_wavelet_name( str.toStdString() );
//    }
//    post( response );
//
//    END_EXECUTE;
//}
