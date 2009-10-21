#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include "view/blipgraphicsitem.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);


//    graphicsView()->setBackgroundBrush(QBrush(QColor(0xd8,0xdc,0xe0)));

//    QGraphicsScene* scene = new QGraphicsScene(this);
//    for( int i = 0; i < 200; ++i )
//    {
//        BlipGraphicsItem* item = new BlipGraphicsItem();
//        item->setPos(30,30);
//    ::rect = scene->addRect(0,0,10,10, QPen(Qt::white), QBrush(Qt::blue));
//    ::item = new QGraphicsTextItem("Hallo neue Welt", ::rect);
//    ::item->setTextInteractionFlags( Qt::TextEditorInteraction);
//    ::item->setPos(20,20);
//    ::item->setTextWidth(200);
//    QObject::connect(::item->document(), SIGNAL(contentsChanged()), SLOT(onContentsChanged()));
//    scene->addItem(item);
//}
//    graphicsView()->setScene(scene);
//    graphicsView()->setSceneRect(0,0,1000,10000);
//    graphicsView()->setRenderHint(QPainter::Antialiasing, true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QGraphicsView* MainWindow::graphicsView()
{
    return ui->graphicsView;
}

QGraphicsView* MainWindow::graphicsViewHead()
{
    return ui->graphicsViewHead;
}

