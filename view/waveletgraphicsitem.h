#ifndef WAVELETGRAPHICSITEM_H
#define WAVELETGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QRectF>

class WaveletView;
class ParticipantGraphicsItem;
class QGraphicsSimpleTextItem;

class WaveletGraphicsItem : public QGraphicsItem
{
public:
    WaveletGraphicsItem(WaveletView* view);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void setWidth(qreal width);

private:
    WaveletView* m_view;
    QList<ParticipantGraphicsItem*> m_participantItems;
    QRectF m_rect;
    QGraphicsSimpleTextItem* m_titleItem;
};

#endif // WAVELETGRAPHICSITEM_H
