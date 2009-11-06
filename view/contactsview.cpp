#include "contactsview.h"
#include "model/contacts.h"
#include "searchbox.h"
#include "participantlistview.h"
#include "titlebar.h"
#include "bigbar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

ContactsView::ContactsView(Contacts* contacts, QWidget* parent)
        : QWidget(parent), m_contacts(contacts)
{
    setMinimumWidth(180);

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    m_bigBar = new BigBar(this);
    m_searchBox = new SearchBox(m_bigBar);

    QHBoxLayout* l = new QHBoxLayout(m_bigBar);
    l->addWidget(m_searchBox);

    m_listView = new ParticipantListView(this);
    m_titleBar = new TitleBar(this);
    m_verticalLayout->addWidget(m_titleBar);
    m_verticalLayout->addWidget(m_bigBar);
    m_verticalLayout->addWidget(m_listView);

    m_titleBar->setText(tr("Contacts"));

    m_listView->setParticipants(contacts->participants());
    connect( contacts, SIGNAL(participantAdded(Participant*)), SLOT(addParticipant(Participant*)));
    connect( contacts, SIGNAL(participantRemoved(Participant*)), SLOT(removeParticipant(Participant*)));

    m_listView->connect( m_searchBox, SIGNAL(textChanged(QString)), SLOT(setFilter(QString)));
}

void ContactsView::addParticipant(Participant* participant)
{
    m_listView->addParticipant(participant);
}

void ContactsView::removeParticipant(Participant* participant)
{
    m_listView->removeParticipant(participant);
}
