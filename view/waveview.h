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

/**
  * Displays a wave and allows the user to edit the wave.
  * Most of the work is done in WaveletView and its affiliated classes, however.
  * The purpose of WaveView is to act as a container for all this detailed stuff and to provide a UI
  * for editing and formatting text (bold, italic, inserting images, gadgets etc.).
  */
class WaveView : public QWidget
{
    Q_OBJECT
public:
    WaveView(Wave* wave, QWidget* parent = 0);
    ~WaveView();

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
    /**
      * Emitted when a new wave with a participant of this wavelet should be created.
      */
    void newWave(Participant*);

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    Wave* m_wave;
    WaveletView* m_waveletView;
    TitleBar* m_titleBar;
    BigBar* m_bigBar;
    ToolBar* m_toolBar;
    QVBoxLayout *m_verticalLayout;
    WaveletGraphicsItem* m_gfx;

    ButtonGraphicsItem* m_boldButton;
    ButtonGraphicsItem* m_italicButton;
    ButtonGraphicsItem* m_underlineButton;
    ButtonGraphicsItem* m_strikeoutButton;
    ButtonGraphicsItem* m_imageButton;
    ButtonGraphicsItem* m_gadgetButton;
};

#endif
