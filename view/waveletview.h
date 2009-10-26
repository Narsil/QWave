#ifndef WAVELETVIEW_H
#define WAVELETVIEW_H

#include <QObject>
#include <QHash>
#include <QString>

class Wavelet;
class WaveView;
class WaveletGraphicsItem;
class BlipGraphicsItem;
class QGraphicsScene;

class Blip;

class WaveletView : public QObject
{
    Q_OBJECT
public:
    WaveletView(WaveView* parent, Wavelet* wavelet);
    ~WaveletView();

    Wavelet* wavelet() { return m_wavelet; }
    QGraphicsScene* scene();
    QGraphicsScene* headScene();

    void fitToWidth( qreal headWidth, qreal width );
    void layoutBlips();

    void setWavelet( Wavelet* wavelet );

private:
    void layoutBlip(Blip* blip, qreal& xoffset, qreal& yoffset, qreal width);
    void layoutBlips(qreal width);

    Wavelet* m_wavelet;
    WaveletGraphicsItem* m_gfx;
    QHash<QString,BlipGraphicsItem*> m_blipItems;
    qreal m_lastWidth;
};

#endif
