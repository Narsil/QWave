#include "contactsview.h"
#include "model/contacts.h"
#include "searchbox.h"
#include "participantlistview.h"

#include <QVBoxLayout>

ContactsView::ContactsView(Contacts* contacts, QWidget* parent)
        : QWidget(parent), m_contacts(contacts)
{
    setMinimumWidth(180);

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    m_searchBox = new SearchBox(this);
    m_listView = new ParticipantListView(this);
    m_verticalLayout->addWidget(m_searchBox);
    m_verticalLayout->addWidget(m_listView);

    m_listView->setParticipants(contacts->participants());
    connect( contacts, SIGNAL(participantAdded(Participant*)), SLOT(addParticipant(Participant*)));
    connect( contacts, SIGNAL(participantRemoved(Participant*)), SLOT(removeParticipant(Participant*)));
}

void ContactsView::addParticipant(Participant* participant)
{
    m_listView->addParticipant(participant);
}

void ContactsView::removeParticipant(Participant* participant)
{
    m_listView->removeParticipant(participant);
}
