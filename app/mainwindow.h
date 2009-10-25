#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QGraphicsItem>
#include <QRectF>

class Environment;

namespace Ui
{
    class MainWindowClass;
}

class QGraphicsView;
class QGraphicsTextItem;
class QTextDocument;
class QPixmap;

class WaveListView;
class WaveView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Environment* environment, QWidget *parent = 0);
    ~MainWindow();

    Environment* environment() const { return m_environment; }
    void setInboxView( WaveListView* view ) { m_inboxView = view; }
    void setWaveView( WaveView* view ) { m_waveView = view; }

private slots:
    void showServerSettings();
    void newWave();

private:
    Ui::MainWindowClass *ui;
    Environment* m_environment;
    WaveListView* m_inboxView;
    WaveView* m_waveView;
};

#endif // MAINWINDOW_H
