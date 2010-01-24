#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QtGui/QDialog>

class Environment;

namespace Ui {
    class ServerSettingsDialog;
}

class ServerSettingsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ServerSettingsDialog)
public:
    explicit ServerSettingsDialog(Environment* environment, QWidget *parent = 0);
    virtual ~ServerSettingsDialog();

protected:
    virtual void changeEvent(QEvent *e);
    virtual void accept();

private:
    Ui::ServerSettingsDialog *m_ui;
    Environment* m_environment;
};

#endif // SERVERSETTINGSDIALOG_H
