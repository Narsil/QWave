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

ParticipantInfoDialog::ParticipantInfoDialog(Participant* participant, QWidget* parent, bool showRemove)
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
    int buttonRight = width() - 10;
    QPushButton* button = new QPushButton();
    button->setText(tr("New Wave"));
    QGraphicsProxyWidget* item = scene()->addWidget(button);
    item->setParentItem( widget() );
    item->setPos( buttonRight - item->preferredWidth(), height() - 10 - item->preferredHeight() );
    buttonRight -= item->preferredWidth() + 10;

    //Adds remove button if parent is a waveview
    if (showRemove)
    {
        QPushButton* removeFromWaveButton = new QPushButton();
        removeFromWaveButton->setText(tr("Remove"));
        QGraphicsProxyWidget* removeItem = scene()->addWidget(removeFromWaveButton);
        removeItem->setParentItem( widget() );
        removeItem->setPos( buttonRight - removeItem->preferredWidth(), height() - 10 - removeItem->preferredHeight() );
        buttonRight -= removeItem->preferredWidth() + 10;
        connect( removeFromWaveButton, SIGNAL(clicked()),SLOT(removeParticipant()));
    }

    QPushButton* button2 = new QPushButton();
    button2->setText(tr("Close"));
    QGraphicsProxyWidget* item2 = scene()->addWidget(button2);
    item2->setParentItem( widget() );
    item2->setPos( buttonRight - item2->preferredWidth(), height() - 10 - item2->preferredHeight() );
    QGraphicsTextItem* text = new QGraphicsTextItem( participant->name(), widget() );
    text->setPos( 110, 10 );
    text->setFont( QFont( "Arial", 18, QFont::Bold ) );
    text = new QGraphicsTextItem( tr("Address") + ": " + participant->address(), widget() );
    text->setPos( 110, 50 );
    text->setFont( QFont( "Arial", 11 ) );

    connect( button, SIGNAL(clicked()), SLOT(newWave()));
    connect( button2, SIGNAL(clicked()), SLOT(close()));
}

void ParticipantInfoDialog::newWave()
{
    emit newWave(m_participant);
    accept();
}

void ParticipantInfoDialog::removeParticipant()
{
    QString address = m_participant->address();
    emit removeParticipant(address);
    accept();
}
