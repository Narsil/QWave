#ifndef INSERTIMAGEDIALOG_H
#define INSERTIMAGEDIALOG_H

#include "popupdialog.h"
#include <QUrl>
#include <QImage>

class QPushButton;
class QLineEdit;
class QNetworkReply;
class QGraphicsPixmapItem;
class Environment;

class InsertImageDialog : public PopupDialog
{
    Q_OBJECT
public:
    InsertImageDialog(Environment* environment, QWidget* parent = 0);

    const QUrl& url() const { return m_url; }
    const QImage& image() const { return m_image; }
    QString caption() const;
    QImage thumbnail() const;

private slots:
    void loadImage();
    void showImage();
    void urlEdited(const QString& text);

private:
    QPushButton* m_insertButton;
    QLineEdit* m_urlEdit;
    QLineEdit* m_captionEdit;
    QUrl m_url;
    QImage m_image;
    QNetworkReply* m_reply;
    QGraphicsPixmapItem* m_pixmapItem;
    Environment* m_environment;
};

#endif // INSERTIMAGEDIALOG_H
