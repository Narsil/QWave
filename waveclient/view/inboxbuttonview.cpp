/*
 * inboxbuttonview.cpp
 *
 *  Created on: Dec 10, 2009
 *      Author: npatry
 */
#include "inboxbuttonview.h"

#include <QPainter>
#include <QLinearGradient>
#include <QColor>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsItem>

InboxButtonView::InboxButtonView(QWidget* parent)
        : QGraphicsView(parent), m_usedWidth(0)
{
    setMinimumHeight( 24 );
    setMaximumHeight( minimumHeight() );

    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setLineWidth(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setAttribute(Qt::WA_Hover, true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scene = new QGraphicsScene(this);
    setScene( m_scene );
}

void InboxButtonView::drawBackground( QPainter* painter, const QRectF& )
{
	QBrush brush2(QColor(0xc9,0xe2,0xfc));
	painter->fillRect(0, 0, width(), height(), brush2);
}

void InboxButtonView::addItem( QGraphicsItem* item )
{
    m_scene->addItem( item );
    m_usedWidth += item->boundingRect().width();
}

