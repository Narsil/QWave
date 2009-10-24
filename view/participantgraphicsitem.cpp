#include "participantgraphicsitem.h"
#include "model/participant.h"
#include <QRectF>
#include <QPixmap>
#include <QPainter>

ParticipantGraphicsItem::ParticipantGraphicsItem(Participant* participant, int size, QGraphicsItem* parent)
        : QGraphicsItem(parent), m_participant(participant)
{
    m_size = size;
    m_pixmap = participant->pixmap().scaled(size - 2, size - 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void ParticipantGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->drawPixmap(1, 1, m_pixmap);
    painter->setPen( QPen(QColor(0xb8,0xc6,0xd9) ) );
    painter->drawRect(boundingRect());
}

QRectF ParticipantGraphicsItem::boundingRect() const
{
    return QRectF(0,0,m_size - 1,m_size - 1);
}
