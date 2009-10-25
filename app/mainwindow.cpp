#include "mainwindow.h"
#include "ui_mainwindow.h"
// #include "view/blipgraphicsitem.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


