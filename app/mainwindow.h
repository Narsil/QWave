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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Environment* environment, QWidget *parent = 0);
    ~MainWindow();

    Environment* environment() const { return m_environment; }

private slots:
    void showServerSettings();

private:
    Ui::MainWindowClass *ui;
    Environment* m_environment;
};

#endif // MAINWINDOW_H
