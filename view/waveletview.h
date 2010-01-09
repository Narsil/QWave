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

/**
  * Shows the content of a wavelet (i,e, its blips) as a series of BlipGraphicsItems.
  */
class WaveletView : public QGraphicsView
{
    Q_OBJECT
public:
    WaveletView(WaveView* parent, Wavelet* wavelet);
    ~WaveletView();

    Wavelet* wavelet() { return m_wavelet; }

    void setWavelet( Wavelet* wavelet );
    BlipGraphicsItem* focusBlipItem() const;

public slots:
    /**
      * Called from BlipGraphicsItem when the first line of the blip changes.
      */
    void setTitle( const QString& title );
    /**
      * Called from BlipGraphicsItem when the height of the blip changes or when the
      * view is resized or initialized.
      */
    void layoutBlips();

protected:
    virtual void resizeEvent( QResizeEvent* event );

private slots:
    /**
      * Called when a BlipGraphicsItem receives focus.
      * This function uses the sender() property, thus, it may only be called as a slot from a BlipGraphicsItem.
      * Everything else will crash and would not make much sense anyway.
      */
    void focusIn();

private:
    void layoutBlip(Blip* blip, qreal& xoffset, qreal& yoffset, qreal width);
    void layoutBlips(qreal width);

    Wavelet* m_wavelet;
    QHash<QString,BlipGraphicsItem*> m_blipItems;
    QGraphicsScene* m_scene;
    BlipGraphicsItem* m_focusItem;
};

#endif
