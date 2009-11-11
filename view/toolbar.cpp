#include "toolbar.h"
#include <QPainter>
#include <QLinearGradient>
#include <QColor>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsItem>

ToolBar::ToolBar(QWidget* parent)
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

void ToolBar::drawBackground( QPainter* painter, const QRectF& )
{
    QLinearGradient g(0, 1, 0, 23);
    g.setColorAt(0, QColor(255,255,255));
    g.setColorAt(1, QColor(0xdd,0xdd,0xdd));
    QBrush brush(g);
    painter->fillRect(0, 1, width(), 22, brush);
    painter->setPen( QColor(0xb8, 0xc6, 0xd9) );
    painter->drawLine( 0, height() - 1, width() - 1, height() - 1 );
}

void ToolBar::addItem( QGraphicsItem* item )
{
    m_scene->addItem( item );
    item->setPos( 5 + m_usedWidth, (height() - item->boundingRect().height()) / 2);
    m_usedWidth += item->boundingRect().width();
}
