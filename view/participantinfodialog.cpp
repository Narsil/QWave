#include "participantinfodialog.h"
#include "model/participant.h"

#include <QGraphicsWidget>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QGraphicsLineItem>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

ParticipantInfoDialog::ParticipantInfoDialog(Participant* participant, QWidget* parent)
        : PopupDialog( parent ), m_participant(participant)
{
    setMinimumWidth(400);
    setMinimumHeight(160);
    this->setSizeGripEnabled(false);

    QGraphicsPixmapItem* image = new QGraphicsPixmapItem( participant->pixmap().scaled(80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), widget() );
    image->setPos( 8, 8 );
    QGraphicsLineItem* line = new QGraphicsLineItem( widget() );
    line->setPen( QPen( QColor( 100, 100, 100 ) ) );
    line->setLine( 8, 110, width() - 2 * 8, 110 );
    QPushButton* button = new QPushButton();
    button->setText(tr("New Wave"));
    QGraphicsProxyWidget* item = scene()->addWidget(button);
    item->setParentItem( widget() );
    item->setPos( width() - item->preferredWidth() - 10, height() - 10 - item->preferredHeight() );
    QGraphicsTextItem* text = new QGraphicsTextItem( participant->name(), widget() );
    text->setPos( 110, 10 );
    text->setFont( QFont( "Arial", 18, QFont::Bold ) );
    text = new QGraphicsTextItem( tr("Address") + ": " + participant->address(), widget() );
    text->setPos( 110, 50 );
    text->setFont( QFont( "Arial", 11 ) );

    connect( button, SIGNAL(clicked()), SLOT(newWave()));
}

void ParticipantInfoDialog::newWave()
{
    emit newWave(m_participant);
    accept();
}
