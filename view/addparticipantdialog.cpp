#include "addparticipantdialog.h"
#include "model/contacts.h"
#include "searchbox.h"
#include "participantlistview.h"
#include "titlebar.h"
#include "bigbar.h"
#include "app/environment.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

AddParticipantDialog::AddParticipantDialog(Environment* environment, QWidget* parent)
        : QDialog(parent), m_environment(environment), m_result(0)
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
    m_listView->setSelectable(true);
    m_titleBar = new TitleBar(this);
    m_verticalLayout->addWidget(m_titleBar);
    m_verticalLayout->addWidget(m_bigBar);
    m_verticalLayout->addWidget(m_listView);

    m_titleBar->setText(tr("Add participant"));
    m_listView->setParticipants(environment->contacts()->participants());

    connect( m_listView, SIGNAL(participantSelected(Participant*)), SLOT(accept(Participant*)));
}

void AddParticipantDialog::accept(Participant* participant)
{
    m_result = participant;
    QDialog::accept();
}
