#ifndef WAVEVIEW_H
#define WAVEVIEW_H

#include <QWidget>

class Wave;
class Participant;
class WaveletView;
class MainWindow;
class TitleBar;
class ToolBar;
class BigBar;
class WaveletGraphicsItem;
class BlipGraphicsItem;
class ButtonGraphicsItem;
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
    void boldClicked();
    void italicCicked();
    void underlineClicked();
    void strikeoutClicked();
    void imageClicked();
    void gadgetClicked();

signals:
	void newWave(Participant*);

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    Wave* m_wave;
    WaveletView* m_waveletView;
    TitleBar* m_titleBar;
    BigBar* m_bigBar;
    ToolBar* m_toolBar;
//    QGraphicsScene* m_headScene;
    QVBoxLayout *m_verticalLayout;
//    QGraphicsView *m_graphicsViewHead;
    WaveletGraphicsItem* m_gfx;

    ButtonGraphicsItem* m_addUserButton;
    ButtonGraphicsItem* m_boldButton;
    ButtonGraphicsItem* m_italicButton;
    ButtonGraphicsItem* m_underlineButton;
    ButtonGraphicsItem* m_strikeoutButton;
    ButtonGraphicsItem* m_imageButton;
    ButtonGraphicsItem* m_gadgetButton;
};

#endif
