#ifndef PARTICIPANTGRAPHICSITEM_H
#define PARTICIPANTGRAPHICSITEM_H

#include <QGraphicsItem>

class Participant;

class ParticipantGraphicsItem : public QGraphicsItem
{
public:
    ParticipantGraphicsItem(Participant* participant, QGraphicsItem* parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

private:
    Participant* m_participant;
};

#endif // PARTICIPANTGRAPHICSITEM_H
