#include "echoey.h"
#include "model/wave.h"
#include "model/otprocessor.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/documentmutation.h"
#include "model/participant.h"
#include "model/waveletdelta.h"
#include "model/waveletdeltaoperation.h"


#include <QDebug>

Echoey::Echoey(Environment* environment) :
    Robot(environment)
{
}

void Echoey::participantAdded(Participant *participant,Wavelet* wavelet)
{
    Q_UNUSED(wavelet);
    QString msg("Hello ");
    msg+=participant->address();
    wavelet->createRootBlip(msg);
}

void Echoey::participantRemoved(Participant *participant,Wavelet* wavelet)
{
    Q_UNUSED(wavelet);
    QString msg("Goodbye ");
    msg+=participant->address();
    wavelet->createRootBlip(msg);
}

void Echoey::newBlipAdded(Blip* blip)
{
    m_followUpBlips[blip] = blip->createFollowUpBlip("");
}

void Echoey::blipChanged(Blip *blip, const DocumentMutation& mutation)
{
    if (!m_followUpBlips[blip])
        m_followUpBlips[blip] = blip->createFollowUpBlip("");
    Blip* b = m_followUpBlips[blip];
    b->mutate(mutation);
}
