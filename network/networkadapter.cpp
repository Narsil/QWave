#include "networkadapter.h"
#include "model/documentmutation.h"
#include "app/environment.h"
#include "model/wavelet.h"
#include "model/blip.h"

NetworkAdapter* NetworkAdapter::s1 = 0;
NetworkAdapter* NetworkAdapter::s2 = 0;

NetworkAdapter::NetworkAdapter(QObject* parent)
        : QObject( parent )
{
    if ( !s1 )
        s1 = this;
    else
        s2 = this;
}

void NetworkAdapter::send( const DocumentMutation& mutation, const QString& waveletId, const QString& docId )
{
    if ( this == s1 )
        s2->receive(mutation, waveletId, docId);
    else
        s1->receive(mutation, waveletId, docId);
}

void NetworkAdapter::receive( const DocumentMutation& mutation, const QString& waveletId, const QString& docId )
{
    Wavelet* wavelet = environment()->wavelet(waveletId);
    if (wavelet)
    {
        Blip* blip = wavelet->blip(docId);
        if ( blip )
        {
            blip->receive(mutation);
        }
    }
}

Environment* NetworkAdapter::environment() const
{
    return (Environment*)parent();
}
