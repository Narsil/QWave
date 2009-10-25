#include "serversettingsdialog.h"
#include "ui_serversettingsdialog.h"

#include <QSettings>

ServerSettingsDialog::ServerSettingsDialog(Environment* environment, QWidget *parent) :
    QDialog(parent), m_ui(new Ui::ServerSettingsDialog), m_environment(environment)
{
    m_ui->setupUi(this);

    QSettings settings( "T.Weis", "QWaveClient");
    settings.beginGroup("server");
    m_ui->server->setText( settings.value("serverName", QVariant("localhost") ).toString() );
    int port = settings.value("serverPort", QVariant((int)9876) ).toInt();
    QString str;
    str.setNum(port);
    m_ui->port->setText( str );
    m_ui->userName->setText( settings.value("userName", QVariant("torben") ).toString() );
    m_ui->password->setText( settings.value("password", QVariant("") ).toString() );
    settings.endGroup();
}

ServerSettingsDialog::~ServerSettingsDialog()
{
    delete m_ui;
}

void ServerSettingsDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ServerSettingsDialog::accept()
{
    QSettings settings( "T.Weis", "QWaveClient");
    settings.beginGroup("server");
    settings.setValue( "serverName", QVariant( m_ui->server->text() ) );
    settings.setValue( "serverPort", QVariant( m_ui->port->text().toInt() ) );
    settings.setValue( "userName", QVariant( m_ui->userName->text() ) );
    settings.setValue( "password", QVariant( m_ui->password->text() ) );
    settings.endGroup();

    QDialog::accept();
}
