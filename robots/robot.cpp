#include "robot.h"
#include "model/participant.h"
#include "app/environment.h"
#include "model/wave.h"
#include "model/blip.h"
#include "model/wavelist.h"
#include "model/wavelet.h"
#include "network/networkadapter.h"
#include "model/structureddocument.h"
#include "model/documentmutation.h"

#include <QDebug>

Robot::Robot(Environment* environment)
	:m_environment(environment)
{
    m_environment->configure();
    bool ok;
    ok=connect(m_environment->inbox(),SIGNAL(waveAdded(Wave*)),SLOT(waveAddedOverhead(Wave*)));
    Q_ASSERT(ok);
    ok=connect(m_environment->networkAdapter(),SIGNAL(connectionStatus(QString)),SLOT(setConnectionStatus(QString)));
    Q_ASSERT(ok);
}

void Robot::waveAddedOverhead(Wave* wave)
{
    wave->environment()->networkAdapter()->openWavelet( wave->wavelet() );
    bool ok;
    ok=connect(wave->environment()->networkAdapter(), SIGNAL(waveOpened(Wave*)), SLOT(waveAdded(Wave*)));
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(participantAdded(Participant*,Wavelet*)),SLOT(participantAdded(Participant*,Wavelet*)),Qt::QueuedConnection);
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(participantRemoved(Participant*,Wavelet*)),SLOT(participantRemoved(Participant*,Wavelet*)));
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(newBlipAdded(Blip*)),SLOT(newBlipAddedOverhead(Blip*)));
    Q_ASSERT(ok);
    ok=connect(wave->wavelet(),SIGNAL(conversationChanged(Wavelet*)),SLOT(conversationChanged(Wavelet*)));
    Q_ASSERT(ok);
}

void Robot::newBlipAddedOverhead(Blip* blip)
{
    bool ok;
    ok=connect(blip,SIGNAL(update(Blip*, const DocumentMutation&)),SLOT(blipChanged(Blip*, const DocumentMutation&)));
    Q_ASSERT(ok);
    newBlipAdded(blip);
}

void Robot::newBlipAdded(Blip* blip)
{
    qDebug()<<"New Blip Added" << blip->id();
}

void Robot::blipChanged(Blip* blip, const DocumentMutation &mutation)
{
    Q_UNUSED(mutation);
    qDebug()<<" Blip "<<blip->id()<<" changed";
}

void Robot::conversationChanged(Wavelet* wavelet)
{
    qDebug()<<"Conversation Changed on "<<wavelet->id();
}

void Robot::waveAdded(Wave* wave)
{
    qDebug()<<"wave "<<wave->id()<<" is now opened";
}

void Robot::setConnectionStatus(const QString &status)
{
    qDebug()<<"Connection status :"<<status;
}

void Robot::participantAdded(Participant *participant, Wavelet *wavelet)
{
    qDebug()<<"Participant added "<<participant->address()<< "on "<<wavelet->id();
}

void Robot::participantRemoved(Participant *participant, Wavelet *wavelet)
{
    qDebug()<<"Participant removed "<<participant->address()<< "on "<<wavelet->id();
}

