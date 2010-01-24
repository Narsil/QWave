#ifndef INSERTGADGETDIALOG_H
#define INSERTGADGETDIALOG_H

#include "popupdialog.h"
#include <QUrl>

class QPushButton;
class QLineEdit;
class Environment;

class InsertGadgetDialog : public PopupDialog
{
    Q_OBJECT
public:
    InsertGadgetDialog(Environment* environment, QWidget* parent = 0);
    const QUrl& url() const { return m_url; }

private slots:
    void urlEdited(const QString&);

private:
    QPushButton* m_insertButton;
    QLineEdit* m_urlEdit;
    QUrl m_url;
    Environment* m_environment;
};

#endif // INSERTGADGETDIALOG_H
