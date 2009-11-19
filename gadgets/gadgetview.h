#ifndef GADGETVIEW_H
#define GADGETVIEW_H

#include <QWebView>
#include <QSizeF>

class GadgetAPI;
class GadgetManifest;
class Environment;
class QUrl;

class GadgetView : public QWebView
{
    Q_OBJECT
public:
    GadgetView(const QUrl& url, int width, const QString& id, Environment* environment);

    QSizeF preferredSize() const { return m_preferredSize; }
    QString id() const { return m_id; }

    void adjustHeight( int height );

signals:
    void sizeChangeRequired(GadgetView* view);

private slots:
    void manifestFinished();

private:
    GadgetAPI* m_api;
    GadgetManifest* m_manifest;
    Environment* m_environment;
    QSizeF m_preferredSize;
    QString m_id;
};

#endif // GADGETVIEW_H
