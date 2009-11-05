#ifndef WAVELETVIEW_H
#define WAVELETVIEW_H

#include <QGraphicsView>
#include <QHash>
#include <QString>

class Wavelet;
class WaveView;
class BlipGraphicsItem;
class QGraphicsScene;

class Blip;

class WaveletView : public QGraphicsView
{
    Q_OBJECT
public:
    WaveletView(WaveView* parent, Wavelet* wavelet);
    ~WaveletView();

    Wavelet* wavelet() { return m_wavelet; }
//    QGraphicsScene* scene();
//    QGraphicsScene* headScene();

//    void fitToWidth( qreal headWidth, qreal width );

    void setTitle( const QString& title );

    void setWavelet( Wavelet* wavelet );

public slots:
    void layoutBlips();

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    void layoutBlip(Blip* blip, qreal& xoffset, qreal& yoffset, qreal width);
    void layoutBlips(qreal width);

    Wavelet* m_wavelet;
    QHash<QString,BlipGraphicsItem*> m_blipItems;
    QGraphicsScene* m_scene;
};

#endif
