#ifndef SERVERSETTINGSDIALOG_H
#define SERVERSETTINGSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class ServerSettingsDialog;
}

class ServerSettingsDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ServerSettingsDialog)
public:
    explicit ServerSettingsDialog(QWidget *parent = 0);
    virtual ~ServerSettingsDialog();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::ServerSettingsDialog *m_ui;
};

#endif // SERVERSETTINGSDIALOG_H
