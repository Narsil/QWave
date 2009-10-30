#ifndef WAVELETGRAPHICSITEM_H
#define WAVELETGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QRectF>
#include <QObject>

class Wavelet;
class WaveletView;
class Participant;
class ParticipantGraphicsItem;
class ButtonGraphicsItem;
class QGraphicsSimpleTextItem;

class WaveletGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    WaveletGraphicsItem(WaveletView* view);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void setWidth(qreal width);

    void setWavelet( Wavelet* wavelet );

private slots:
    void addParticipant(Participant* participant);
    void removeParticipant(Participant* participant);
    void showAddParticipantDialog();

private:
    void updateParticipants();

    Wavelet* m_wavelet;
    WaveletView* m_view;
    QList<ParticipantGraphicsItem*> m_participantItems;
    QRectF m_rect;
    ButtonGraphicsItem* m_addUserButton;
};

#endif // WAVELETGRAPHICSITEM_H
