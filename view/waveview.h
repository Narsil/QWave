#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QWidget>

class Wave;
class WaveletView;
class MainWindow;
class QGraphicsScene;
class QVBoxLayout;
class QGraphicsView;

class WaveView : public QWidget
{
    Q_OBJECT
public:
    WaveView(Wave* wave);
    ~WaveView();

    QGraphicsScene* scene() { return m_scene; }
    QGraphicsScene* headScene() { return m_headScene; }

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    Wave* m_wave;
    WaveletView* m_waveletView;
    QGraphicsScene* m_scene;
    QGraphicsScene* m_headScene;
    QVBoxLayout *m_verticalLayout;
    QGraphicsView *m_graphicsViewHead;
    QGraphicsView *m_graphicsView;
};

#endif
