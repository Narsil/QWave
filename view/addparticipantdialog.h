#ifndef ADDPARTICIPANTDIALOG_H
#define ADDPARTICIPANTDIALOG_H

#include <QDialog>

class Environment;
class Participant;
class ParticipantListView;
class SearchBox;
class TitleBar;
class BigBar;
class QVBoxLayout;

class AddParticipantDialog : public QDialog
{
    Q_OBJECT
public:
    AddParticipantDialog(Environment* environment, QWidget* parent = 0);

    Participant* result() const { return m_result; }

private slots:
    void accept(Participant* participant);
    void returnPressed();

private:
    Environment* m_environment;
    BigBar* m_bigBar;
    TitleBar* m_titleBar;
    SearchBox* m_searchBox;
    ParticipantListView* m_listView;
    QVBoxLayout* m_verticalLayout;
    Participant* m_result;
};

#endif // ADDPARTICIPANTDIALOG_H
