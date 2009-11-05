#include "serversettingsdialog.h"
#include "ui_serversettingsdialog.h"
#include "app/environment.h"
#include "app/settings.h"

ServerSettingsDialog::ServerSettingsDialog(Environment* environment, QWidget *parent) :
    QDialog(parent), m_ui(new Ui::ServerSettingsDialog), m_environment(environment)
{
    m_ui->setupUi(this);

    Settings* s = environment->settings();
    m_ui->server->setText( s->serverName() );
    QString str;
    str.setNum(s->serverPort());
    m_ui->port->setText( str );
    m_ui->userName->setText( s->userName() );
    m_ui->userAddress->setText( s->userAddress() );
    m_ui->password->setText( s->password() );
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
    Settings* s = m_environment->settings();

    s->setServerName( m_ui->server->text() );
    s->setServerPort( m_ui->port->text().toInt() );
    s->setUserName( m_ui->userName->text() );
    s->setUserAddress( m_ui->userAddress->text() );
    s->setPassword( m_ui->password->text() );

    QDialog::accept();
}
