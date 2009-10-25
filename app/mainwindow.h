#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QGraphicsItem>
#include <QRectF>

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
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;
};

#endif // MAINWINDOW_H
