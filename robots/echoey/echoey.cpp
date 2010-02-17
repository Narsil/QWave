#include "echoey.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/participant.h"

#include <QDebug>

Echoey::Echoey(Environment* environment) :
    Robot(environment)
{
}

void Echoey::waveAdded(Wave *wave)
{
    wave->wavelet()->createRootBlip("Hello World !");
}

void Echoey::participantAdded(Participant *participant,Wavelet* wavelet)
{
    QString msg("Hello ");
    msg+=participant->address();
//    wavelet->createRootBlip(msg);
}

void Echoey::participantRemoved(Participant *participant,Wavelet* wavelet)
{
    QString msg("Goodbye ");
    msg+=participant->address();
//    wavelet->createRootBlip(msg);
}

void Echoey::conversationChanged(Wavelet* wavelet)
{
//    wavelet->createRootBlip("Hello World ! ");
}

void Echoey::onBlipSubmitted(Blip* blip)
{
//    QString text= "Hello World !";
//    text+= blip->id();
//    blip->createFollowUpBlip(text);
}
