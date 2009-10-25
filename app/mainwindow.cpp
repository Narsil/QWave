#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serversettingsdialog.h"

MainWindow::MainWindow(Environment* environment, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass), m_environment(environment)
{
    ui->setupUi(this);

    connect( ui->actionServerSettings, SIGNAL(triggered()), SLOT(showServerSettings()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showServerSettings()
{
    ServerSettingsDialog* dlg = new ServerSettingsDialog(m_environment, this);
    dlg->show();
}
