#include "inboxview.h"
#include "searchbox.h"
#include "titlebar.h"
#include "bigbar.h"
#include "wavelistview.h"
#include "app/environment.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

InboxView::InboxView(Environment* environment, QWidget* parent)
        : QWidget(parent), m_environment( environment )
{
    setMinimumWidth(300);

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    m_bigBar = new BigBar(this);
    m_searchBox = new SearchBox(m_bigBar);
    m_searchBox->setText("in:inbox");
    QHBoxLayout* l = new QHBoxLayout(m_bigBar);
    l->addWidget(m_searchBox);

    m_listView = new WaveListView( environment->inbox() );
    m_listView->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
    m_titleBar = new TitleBar(this);
    m_verticalLayout->addWidget(m_titleBar);
    m_verticalLayout->addWidget(m_bigBar);
    m_verticalLayout->addWidget(m_listView);

    m_titleBar->setText(tr("Inbox"));

    connect( m_listView, SIGNAL(selected(Wave*)), SIGNAL(selected(Wave*)));
}

void InboxView::select( Wave* wave )
{
    m_listView->select(wave);
}
