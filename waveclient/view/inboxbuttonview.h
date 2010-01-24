/*
 * inboxbuttonview.h
 *
 *  Created on: Dec 10, 2009
 *      Author: npatry
 */

#ifndef INBOXBUTTONVIEW_H_
#define INBOXBUTTONVIEW_H_

#include <QGraphicsView>

class InboxButtonView : public QGraphicsView
{
public:
	InboxButtonView(QWidget* parent = 0);

    QGraphicsScene* scene() const { return m_scene; }

    void addItem( QGraphicsItem* item );

protected:
    virtual void drawBackground( QPainter* painter, const QRectF& rect );

private:
    QGraphicsScene* m_scene;
    int m_usedWidth;
};

#endif /* INBOXBUTTONVIEW_H_ */
