#ifndef PARTICIPANTINFODIALOG_H
#define PARTICIPANTINFODIALOG_H

#include "popupdialog.h"

class Participant;

class ParticipantInfoDialog : public PopupDialog
{
    Q_OBJECT
public:
    ParticipantInfoDialog(Participant* participant, QWidget* parent = 0);

signals:
    void newWave(Participant* participant);

private slots:
    void newWave();

private:
    Participant* m_participant;
};

#endif // PARTICIPANTINFODIALOG_H
