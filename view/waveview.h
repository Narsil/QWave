#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QWidget>

class Wave;
class WaveletView;
class MainWindow;
class TitleBar;
class WaveletGraphicsItem;
class BlipGraphicsItem;
class QGraphicsScene;
class QVBoxLayout;
class QGraphicsView;

class WaveView : public QWidget
{
    Q_OBJECT
public:
    WaveView(Wave* wave, QWidget* parent = 0);
    ~WaveView();

//    QGraphicsScene* scene() { return m_scene; }
//    QGraphicsScene* headScene() { return m_headScene; }

    Wave* wave() const { return m_wave; }
    WaveletView* waveletView() const { return m_waveletView; }
    BlipGraphicsItem* focusBlipItem() const;
    void setTitle( const QString& title );

public slots:
    void setWave( Wave* wave );

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    Wave* m_wave;
    WaveletView* m_waveletView;
    TitleBar* m_titleBar;
//    QGraphicsScene* m_scene;
    QGraphicsScene* m_headScene;
    QVBoxLayout *m_verticalLayout;
    QGraphicsView *m_graphicsViewHead;
//    QGraphicsView *m_graphicsView;
    WaveletGraphicsItem* m_gfx;
};

#endif
