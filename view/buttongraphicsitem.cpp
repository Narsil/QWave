#include "buttongraphicsitem.h"

ButtonGraphicsItem::ButtonGraphicsItem(const QPixmap& pixmap, QGraphicsItem* parent)
        : QObject(), QGraphicsPixmapItem(pixmap, parent)
{
}

void ButtonGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent* )
{
    emit clicked();
}
