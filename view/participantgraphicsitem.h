#ifndef PARTICIPANTGRAPHICSITEM_H
#define PARTICIPANTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPixmap>
#include <QObject>

class Participant;

class ParticipantGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    ParticipantGraphicsItem(Participant* participant, int size, bool showName, QGraphicsItem* parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    Participant* participant() const { return m_participant; }

    void setSelectable(bool flag);
    void setWidth(int width);

signals:
    void clicked( Participant* participan );

protected:
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private:
    Participant* m_participant;
    int m_size;
    int m_textWidth;
    int m_textHeight;
    QPixmap m_pixmap;
    bool m_showName;
    bool m_hover;
    int m_width;
};

#endif // PARTICIPANTGRAPHICSITEM_H
