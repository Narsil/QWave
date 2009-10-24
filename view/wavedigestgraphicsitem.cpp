#include "wavedigestgraphicsitem.h"
#include "participantgraphicsitem.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/participant.h"

#include <QPen>
#include <QPainter>
#include <QGraphicsTextItem>

WaveDigestGraphicsItem::WaveDigestGraphicsItem(Wave* wave, int width, QGraphicsItem* parent)
        : QObject(), QGraphicsItem(parent), m_wave(wave), m_rect(0 ,0, width, 38), m_hover(false)
{    
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    connect(wave, SIGNAL(digestChanged()), SLOT(updateDigest()));
    connect(wave->wavelet(), SIGNAL(participantAdded(Participant*)), SLOT(addParticipant(Participant*)));
    connect(wave->wavelet(), SIGNAL(participantRemoved(Participant*)), SLOT(removeParticipant(Participant*)));

    foreach( Participant* p, wave->wavelet()->participants() )
    {
        addParticipant(p);
    }

    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setPos( 4 + 32 * 3, 1 );
    m_textItem->setTextWidth(width - m_textItem->x() - 4);
    m_textItem->setPlainText( wave->digest() );
}

void WaveDigestGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if ( isSelected() )
    {
        painter->fillRect( boundingRect(), QBrush(QColor(0xe0, 0xe8, 0xa4)));
        painter->setPen(QColor(0xce,0xd6,0x97));
        painter->drawLine(0, m_rect.height() - 1, m_rect.width() - 1, m_rect.height() - 1);
    }
    else if ( m_hover )
    {
        painter->fillRect( boundingRect(), QBrush(QColor(0xee,0xee,0xee)));
    }
    else
    {
        painter->setPen(QColor(0xee,0xee,0xee));
        painter->drawLine(0, m_rect.height() - 1, m_rect.width() - 1, m_rect.height() - 1);
    }
}

QRectF WaveDigestGraphicsItem::boundingRect() const
{
    return m_rect;
}

void WaveDigestGraphicsItem::setWidth( int width )
{
    m_rect = QRectF(0 ,0, width, 38);
    m_textItem->setTextWidth(width - m_textItem->x() - 4);
    this->prepareGeometryChange();
}

void WaveDigestGraphicsItem::updateDigest()
{
    m_textItem->setPlainText( m_wave->digest() );
}

void WaveDigestGraphicsItem::addParticipant(Participant* participant)
{
    if ( m_participants.count() >= 3 )
        return;
    ParticipantGraphicsItem* item = new ParticipantGraphicsItem(participant, 28, this);
    item->setPos( 4 + 32 * m_participants.count(), 4 );
    m_participants.append(item);
}

void WaveDigestGraphicsItem::removeParticipant(Participant* participant)
{
    bool isshown = false;
    foreach( ParticipantGraphicsItem* item, m_participants )
    {
        if ( item->participant() == participant )
            isshown = true;
    }
    if ( !isshown )
        return;

    foreach( ParticipantGraphicsItem* item2, m_participants )
    {
        delete item2;
    }
    m_participants.clear();

    for( int i = 0; i < qMin(3, m_wave->wavelet()->participants().count()); ++i )
    {
        addParticipant( m_wave->wavelet()->participants()[i] );
    }
}

void WaveDigestGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = true;
    update();
}

void WaveDigestGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = false;
    update();
}

void WaveDigestGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent * )
{
    emit clicked(this);
}
