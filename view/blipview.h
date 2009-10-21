#ifndef BLIPLETVIEW_H
#define BLIPLETVIEW_H

#include <QObject>

class Blip;
class WaveletView;
class BlipGraphicsItem;
class QGraphicsTextItem;
class QGraphicsScene;

class BlipView : public QObject
{
    Q_OBJECT
public:
    BlipView( WaveletView* parent, Blip* blip );
    ~BlipView();

    QGraphicsScene* scene();

    Blip* blip() { return m_blip; }

private:
    Blip* m_blip;
    BlipGraphicsItem* m_gfx;
};

#endif
