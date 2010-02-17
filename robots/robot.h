#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include "app/environment.h"
#include "model/participant.h"

class Wave;
class Participant;
class Wavelet;
class Blip;

class Robot:public QObject
{
    Q_OBJECT
public:
    Robot(Environment* environment);
    QString address() { return m_environment->localUser()->address();}
protected slots:
    /**
      *  Not triggered at the moment
      *  Will be triggered when the wave is opened
      */
    virtual void waveAdded(Wave* wave);

    /**
      * Triggered when incoming wave arrive,
      * if you derive it, reimplement connections.
      */
    virtual void wavedigestAdded(Wave * wave);

    /**
      * triggered when connection status is modified.( Not enabled yet)
      */
    virtual void setConnectionStatus(const QString &status);

    /**
      * triggered when a participant is added to a wavelet
      * including the robot itself and the previous participants
      */
    virtual void participantAdded(Participant* participant,Wavelet* wavelet);

    /**
      * triggered when a participant is added to a wavelet
      */
    virtual void participantRemoved(Participant* participant,Wavelet* wavelet);

    /**
      * triggered when the conversation is update (that means
      * new blip for example.
      */
    virtual void conversationChanged(Wavelet* wavelet);

    /**
      * triggered on a new blip. Buggy and random
      * behaviour
      */
    virtual void onBlipSubmitted(Blip* blip);
protected:
    Environment* m_environment;
};

#endif
