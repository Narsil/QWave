#include "insertgadgetdialog.h"
#include "app/environment.h"

#include <QGraphicsWidget>
#include <QGraphicsTextItem>
#include <QPushButton>
#include <QGraphicsLineItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QPen>
#include <QLineEdit>

#define IMAGE_SIZE 120

InsertGadgetDialog::InsertGadgetDialog(Environment* environment, QWidget* parent)
        : PopupDialog( parent ), m_environment(environment)
{
    setMinimumWidth(500);
    setMinimumHeight(100);
    this->setSizeGripEnabled(false);

    QGraphicsLineItem* line = new QGraphicsLineItem( widget() );
    line->setPen( QPen( QColor( 100, 100, 100 ) ) );
    line->setLine( 8, 150, width() - 2 * 8, 150 );

    QGraphicsTextItem*  text = new QGraphicsTextItem( tr("Gadget URL"), widget() );
    text->setPos( 10, 10 );

    m_urlEdit = new QLineEdit();
    m_urlEdit->setText(tr("http://"));
    m_urlEdit->setMinimumWidth( 480 );
    QGraphicsProxyWidget* item3 = scene()->addWidget(m_urlEdit);
    item3->setParentItem( widget() );
    item3->setPos( 10, 32 );

    QPushButton* button = new QPushButton();
    button->setText(tr("Cancel"));
    QGraphicsProxyWidget* item = scene()->addWidget(button);
    item->setParentItem( widget() );
    item->setPos( width() - item->preferredWidth() - 10, height() - 10 - item->preferredHeight() );
    connect( button, SIGNAL(clicked()), SLOT(reject()));

    m_insertButton = new QPushButton();
    m_insertButton->setText(tr("Insert"));
    m_insertButton->setEnabled(false);
    QGraphicsProxyWidget* item2 = scene()->addWidget(m_insertButton);
    item2->setParentItem( widget() );
    item2->setPos( width() - item->preferredWidth() - 10 - item2->preferredWidth() - 8, height() - 10 - item->preferredHeight() );

    connect( m_insertButton, SIGNAL(clicked()), SLOT(accept()));
    connect( m_urlEdit, SIGNAL(textEdited(QString)), SLOT(urlEdited(QString)));
}

void InsertGadgetDialog::urlEdited(const QString&)
{
    m_url = QUrl(m_urlEdit->text());
    //TODO Check gadget is valid
    if(m_url.isEmpty() or !m_url.isValid())
    	m_insertButton->setEnabled(false);
    else
    	m_insertButton->setEnabled(true);
}

