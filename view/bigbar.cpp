#include "bigbar.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QGraphicsScene>

BigBar::BigBar(QWidget* parent)
        : QGraphicsView( parent )
{
    setMinimumHeight( 42 + 2*5 + 1 );
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

void BigBar::drawBackground( QPainter* painter, const QRectF& rect )
{
    QBrush brush2(QColor(0xc9,0xe2,0xfc));
    painter->fillRect(0, 0, width(), height(), brush2);
    painter->setPen( QColor(0xb8, 0xc6, 0xd9) );
    painter->drawLine( 0, height() - 1, width() - 1, height() - 1 );
}

