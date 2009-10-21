#include "participantgraphicsitem.h"
#include "model/participant.h"
#include <QRectF>
#include <QPixmap>
#include <QPainter>

ParticipantGraphicsItem::ParticipantGraphicsItem(Participant* participant, QGraphicsItem* parent)
        : QGraphicsItem(parent), m_participant(participant)
{
}

void ParticipantGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->drawPixmap(1, 1, m_participant->pixmap());
    painter->setPen( QPen(QColor(0xb8,0xc6,0xd9) ) );
    painter->drawRect(boundingRect());
}

QRectF ParticipantGraphicsItem::boundingRect() const
{
    return QRectF(0,0,41,41);
}
