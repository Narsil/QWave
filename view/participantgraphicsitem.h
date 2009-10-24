#ifndef PARTICIPANTGRAPHICSITEM_H
#define PARTICIPANTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPixmap>

class Participant;

class ParticipantGraphicsItem : public QGraphicsItem
{
public:
    ParticipantGraphicsItem(Participant* participant, int size, QGraphicsItem* parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    Participant* participant() const { return m_participant; }

private:
    Participant* m_participant;
    int m_size;
    QPixmap m_pixmap;
};

#endif // PARTICIPANTGRAPHICSITEM_H
