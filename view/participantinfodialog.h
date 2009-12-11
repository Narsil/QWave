#ifndef PARTICIPANTINFODIALOG_H
#define PARTICIPANTINFODIALOG_H

#include "popupdialog.h"

class Participant;

/**
  * Shows information about a participant, i.e. a picture, name, etc.
  */
class ParticipantInfoDialog : public PopupDialog
{
    Q_OBJECT
public:
    ParticipantInfoDialog(Participant* participant, QWidget* parent = 0, bool showRemove=false);

signals:
    void newWave(Participant* participant);
    void removeParticipant(const QString& adress);

private slots:
    void newWave();
    void removeParticipant();

private:
    Participant* m_participant;
};

#endif // PARTICIPANTINFODIALOG_H
