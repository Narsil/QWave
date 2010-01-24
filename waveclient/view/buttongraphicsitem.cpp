#include "buttongraphicsitem.h"

ButtonGraphicsItem::ButtonGraphicsItem(const QPixmap& pixmap, QGraphicsItem* parent, qreal hoverOpacity)
        : QObject(), QGraphicsPixmapItem(pixmap, parent), m_hoverOpacity(hoverOpacity)
{
	setAcceptHoverEvents(true);
}

void ButtonGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent* )
{
    emit clicked();
}

void ButtonGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
	this->setOpacity(m_hoverOpacity);
}

void ButtonGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    this->setOpacity(1);
}
