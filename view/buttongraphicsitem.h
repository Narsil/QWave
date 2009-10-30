#ifndef BUTTONGRAPHICSITEM_H
#define BUTTONGRAPHICSITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>

class ButtonGraphicsItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    ButtonGraphicsItem(const QPixmap& pixmap, QGraphicsItem* parent = 0);

signals:
    void clicked();

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

protected:
};

#endif // BUTTONGRAPHICSITEM_H
