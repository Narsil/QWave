#ifndef BUTTONGRAPHICSITEM_H
#define BUTTONGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPainter>

class ButtonGraphicsItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    ButtonGraphicsItem(const QPixmap& pixmap, QGraphicsItem* parent = 0, qreal hoverOpacity = 0.7);

signals:
    void clicked();

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );

protected:
	qreal m_hoverOpacity;

};

#endif // BUTTONGRAPHICSITEM_H
