#ifndef CONTACTSVIEW_H
#define CONTACTSVIEW_H

#include <QWidget>

class Contacts;
class Participant;
class ParticipantListView;
class SearchBox;
class TitleBar;
class BigBar;
class QVBoxLayout;

class ContactsView : public QWidget
{
    Q_OBJECT
public:
    ContactsView(Contacts* contacts, QWidget* parent = 0);

    Contacts* contacts() const { return m_contacts; }

private slots:
    void addParticipant(Participant* participant);
    void removeParticipant(Participant* participant);

private:
    Contacts* m_contacts;
    ParticipantListView* m_listView;
    SearchBox* m_searchBox;
    TitleBar* m_titleBar;
    BigBar* m_bigBar;
    QVBoxLayout* m_verticalLayout;
};

#endif // CONTACTSVIEW_H
