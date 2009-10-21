#include "waveletgraphicsitem.h"
#include "waveletview.h"
#include "model/wavelet.h"
#include "participantgraphicsitem.h"

#include <QPainter>
#include <QBrush>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QGraphicsScene>

WaveletGraphicsItem::WaveletGraphicsItem(WaveletView* view)
       : m_view(view)
{    
    m_titleItem = new QGraphicsSimpleTextItem("Title", this);
    m_titleItem->setPos(6, 3);
    m_titleItem->setBrush(QBrush(Qt::white));

    qreal dx = 0;
    qreal dy = 2 *3 + m_titleItem->boundingRect().height();
    foreach( Participant* p, view->wavelet()->participants() )
    {
        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(p, this);
        item->setPos(dx + 5, dy + 5);
        dx += item->boundingRect().width() + 5;
        m_participantItems.append( item );
    }

    dy += 42 + 2 * 5;
    dy += 1 + 22 + 1;

    m_rect = QRectF( 0, 0, 100, dy);
}

void WaveletGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    qreal dy = 0;
    QBrush brush(QColor(0x55,0x90,0xd2));
    painter->fillRect(0, 0, m_rect.width(), m_titleItem->boundingRect().height() + 2 * 3, brush);
    dy += 2 *3 + m_titleItem->boundingRect().height();

    QBrush brush2(QColor(0xc9,0xe2,0xfc));
    painter->fillRect(0, dy, m_rect.width(), 42 + 2 * 5, brush2);
    dy += 42 + 2 * 5;

    painter->setPen(QPen(QColor(0xaa,0xaa,0xaa)));
    painter->drawLine(0, dy, m_rect.width(), dy);
    painter->drawLine(0, dy + 23, m_rect.width(), dy + 23);

    QLinearGradient g(0, dy+1, 0, dy+23);
    g.setColorAt(0, QColor(255,255,255));
    g.setColorAt(1, QColor(0xdd,0xdd,0xdd));
    QBrush brush3(g);
    painter->fillRect(0, dy + 1, m_rect.width(), 22, brush3);
    dy += 1 + 22 + 1;
}

QRectF WaveletGraphicsItem::boundingRect() const
{
    return m_rect;
}

void WaveletGraphicsItem::setWidth(qreal width)
{
    m_rect.setWidth(width);
    this->prepareGeometryChange();
}
