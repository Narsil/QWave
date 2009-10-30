#include "participantgraphicsitem.h"
#include "model/participant.h"
#include <QRectF>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>

ParticipantGraphicsItem::ParticipantGraphicsItem(Participant* participant, int size, bool showName, QGraphicsItem* parent)
        : QGraphicsItem(parent), m_participant(participant), m_showName(showName), m_hover(false), m_width(-1)
{
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    m_size = size;
    m_pixmap = participant->pixmap().scaled(size - 2, size - 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QFont font;
    QFontMetrics metrics(font);
    QRect r = metrics.boundingRect(m_participant->name());
    m_textWidth = r.width();
    m_textHeight = r.height();
}

void ParticipantGraphicsItem::setSelectable(bool flag)
{
    if ( flag )
    {
        this->setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
    else
    {
        // TODO
    }
}

void ParticipantGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF rect = boundingRect();

    if ( isSelected() )
    {
        painter->fillRect( rect, QBrush(QColor(0xe0, 0xe8, 0xa4)));
        painter->setPen(QColor(0xce,0xd6,0x97));
        painter->drawLine(0, rect.height() - 1, rect.width() - 1, rect.height() - 1);
    }
    else if ( m_hover )
    {
        painter->fillRect( rect, QBrush(QColor(0xee,0xee,0xee)));
    }

    painter->drawPixmap(1, 1, m_pixmap);
    painter->setPen( QPen(QColor(0xb8,0xc6,0xd9) ) );
    painter->drawRect(QRectF(0,0,m_size-1,m_size-1));

    if ( m_showName )
    {
        painter->setPen( QPen(QColor(0,0,0) ) );
        painter->drawText(m_pixmap.width() + 6, m_textHeight, m_participant->name() );
    }

}

QRectF ParticipantGraphicsItem::boundingRect() const
{    
    if ( m_showName )
    {
        if ( m_width != -1 )
            return QRectF(0,0, m_width, qMax(m_size, m_textHeight));
        else
            return QRectF(0,0, m_size + 6 + m_textWidth, qMax(m_size, m_textHeight));
    }
    else
    {
        if ( m_width != -1 )
            return QRectF(0,0,m_width,m_size);
        else
            return QRectF(0,0,m_size,m_size);
    }
}

void ParticipantGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = true;
    update();
}

void ParticipantGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = false;
    update();
}

void ParticipantGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent * )
{
    emit clicked(m_participant);
}

void ParticipantGraphicsItem::setWidth(int width)
{
    if ( m_width == width )
        return;
    m_width = width;
    this->prepareGeometryChange();
}
