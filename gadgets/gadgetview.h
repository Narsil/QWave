#ifndef GADGETVIEW_H
#define GADGETVIEW_H

#include <QWebView>
#include <QSizeF>

class GadgetAPI;
class GadgetManifest;
class Environment;
class Blip;
class QUrl;

class GadgetView : public QWebView
{
    Q_OBJECT
public:
    /**
      * @param blip is the blip to which the gadget belongs.
      * @param url is the URL of the gadget manifest.
      * @param id is an internal Id which uniquely identifies the gadget inside a blip.
      */
    GadgetView(Blip* blip, const QUrl& url, int width, const QString& id, Environment* environment);

    QSizeF preferredSize() const { return m_preferredSize; }
    QString id() const { return m_id; }

    Environment* environment() const { return m_environment; }
    Blip* blip() const { return m_blip; }

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
    Blip* m_blip;
};

#endif // GADGETVIEW_H
