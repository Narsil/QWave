#ifndef ECHOEY_H
#define ECHOEY_H
#include "robot.h"

#include <QHash>

class Environment;
class Participant;
class Wavelet;
class Blip;

class Echoey : public Robot
{
Q_OBJECT
public:
    Echoey(Environment* environment);
protected slots:
    virtual void participantAdded(Participant* participant,Wavelet* wavelet);
    virtual void participantRemoved(Participant* participant,Wavelet* wavelet);
    virtual void newBlipAdded(Blip* blip);
    virtual void blipChanged(Blip* blip, const DocumentMutation& mutation);
protected:
    QHash<Blip*,Blip*> m_followUpBlips;

};

#endif // ECHOEY_H
