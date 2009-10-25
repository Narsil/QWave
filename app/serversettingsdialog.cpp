#include "serversettingsdialog.h"
#include "ui_serversettingsdialog.h"

ServerSettingsDialog::ServerSettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ServerSettingsDialog)
{
    m_ui->setupUi(this);
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
