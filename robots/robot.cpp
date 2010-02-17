#include "robot.h"
#include "model/participant.h"
#include "app/environment.h"
#include "model/wave.h"
#include "model/wavelist.h"
#include "model/wavelet.h"
#include "network/networkadapter.h"
#include "model/structureddocument.h"

#include <QDebug>

Robot::Robot(Environment* environment)
	:m_environment(environment)
{
    m_environment->configure();
    bool ok;
    ok=connect(m_environment->inbox(),SIGNAL(waveAdded(Wave*)),SLOT(wavedigestAdded(Wave*)));
    Q_ASSERT(ok);
//    ok=connect(m_environment->networkAdapter(),SIGNAL(connectionStatus(QString)),SLOT(setConnectionStatus(QString)));
//    Q_ASSERT(ok);
}

void Robot::wavedigestAdded(Wave* wave)
{
    wave->environment()->networkAdapter()->openWavelet( wave->wavelet() );
    bool ok;
    ok=connect(wave->wavelet(),SIGNAL(participantAdded(Participant*,Wavelet*)),SLOT(participantAdded(Participant*,Wavelet*)),Qt::QueuedConnection);
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(participantRemoved(Participant*,Wavelet*)),SLOT(participantRemoved(Participant*,Wavelet*)));
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(newBlipSubmitted(Blip*)),SLOT(onBlipSubmitted(Blip*)));
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(conversationChanged(Wavelet*)),SLOT(conversationChanged(Wavelet*)));
    Q_ASSERT(ok);
}

void Robot::onBlipSubmitted(Blip* blip)
{
    Q_UNUSED(blip);

    qDebug()<<"New Blip Submitted";
}

void Robot::conversationChanged(Wavelet* wavelet)
{
    Q_UNUSED(wavelet);

    qDebug()<<"Conversation Changed";
}

void Robot::waveAdded(Wave* wave)
{
    qDebug()<<"new Wave "<<wave->id();
}

void Robot::setConnectionStatus(const QString &status)
{
    qDebug()<<status;
}

void Robot::participantAdded(Participant *participant, Wavelet *wavelet)
{
    qDebug()<<"Participant added "<<participant->address()<< "on "<<wavelet->id();
}

void Robot::participantRemoved(Participant *participant, Wavelet *wavelet)
{
    qDebug()<<"Participant removed "<<participant->address()<< "on "<<wavelet->id();
}

