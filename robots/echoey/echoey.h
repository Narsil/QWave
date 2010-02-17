#ifndef ECHOEY_H
#define ECHOEY_H
#include "robot.h"

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
    virtual void waveAdded(Wave *wave);
    virtual void participantAdded(Participant* participant,Wavelet* wavelet);
    virtual void participantRemoved(Participant* participant,Wavelet* wavelet);
    virtual void onBlipSubmitted(Blip* blip);
    virtual void conversationChanged(Wavelet *wavelet);
};

#endif // ECHOEY_H
