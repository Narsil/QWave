#ifndef WAVEDIGESTGRAPHICSITEM_H
#define WAVEDIGESTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QRectF>
#include <QList>

class Wave;
class ParticipantGraphicsItem;
class Participant;
class QGraphicsTextItem;

class WaveDigestGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    WaveDigestGraphicsItem(Wave* wave, int width, QGraphicsItem* parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void setWidth( int width );

    Wave* wave() const { return m_wave; }

protected:
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    void repaint();
    void updateDigest();
    void addParticipant(Participant* participant);
    void removeParticipant(Participant* participant);

signals:
    void clicked(WaveDigestGraphicsItem*);

private:
    Wave* m_wave;
    QRectF m_rect;
    QList<ParticipantGraphicsItem*> m_participants;
    QGraphicsTextItem* m_textItem;
    bool m_hover;       
    int m_blipCount;
    int m_unreadBlipCount;
};

#endif // WAVEDIGESTGRAPHICSITEM_H
