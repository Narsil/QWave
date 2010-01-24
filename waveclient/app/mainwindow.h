#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QGraphicsItem>
#include <QRectF>

class Environment;
class Participant;
class QLabel;

namespace Ui
{
    class MainWindowClass;
}

class QGraphicsView;
class QGraphicsTextItem;
class QTextDocument;
class QPixmap;

class InboxView;
class WaveView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Environment* environment, QWidget *parent = 0);
    ~MainWindow();

    Environment* environment() const { return m_environment; }
    void setInboxView(InboxView* view ) { m_inboxView = view; }
    void setWaveView( WaveView* view ) { m_waveView = view; }

private slots:
    void setConnectionStatus( const QString& status );
    void showServerSettings();
    void newWave();
    void newWave(Participant* p);

private:
    Ui::MainWindowClass *ui;
    Environment* m_environment;
    InboxView* m_inboxView;
    WaveView* m_waveView;
    QLabel* m_connectionStatus;
};

#endif // MAINWINDOW_H
