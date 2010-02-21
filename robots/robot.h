#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include "app/environment.h"
#include "model/participant.h"

class Wave;
class Participant;
class Wavelet;
class Blip;
class DocumentMutation;

class Robot:public QObject
{
    Q_OBJECT
public:
    Robot(Environment* environment);
    QString address() { return m_environment->localUser()->address();}
protected slots:
    /**
      * triggered when wave is opened but as document
      * is not updated yet, reacting withing this slot
      * usually produces errors. Need history error
      * corrections.
      */
    virtual void waveAdded(Wave* wave);

    /**
      * Triggered when incoming wave arrive,
      * if you derive it, reimplement connections.
      */
    virtual void waveAddedOverhead(Wave * wave);

    /**
      * triggered when connection status is modified.s
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
      * triggered on a new blip.
      */
    virtual void newBlipAdded(Blip* blip);

    /**
      * triggered on new blip, connects blip signals to robot
      * use preferably newBlipAdded
      */
    virtual void newBlipAddedOverhead(Blip* blip);

    /**
      * triggered whenever a blip has been modified
      */
    virtual void blipChanged(Blip* blip, const DocumentMutation& mutation);
protected:
    Environment* m_environment;
};

#endif
