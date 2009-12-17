#ifndef GADGETVIEW_H
#define GADGETVIEW_H

#include <QWebView>
#include <QSizeF>

class GadgetAPI;
class GadgetManifest;
class Environment;
class Blip;
class QUrl;

/**
  * Show the GUI of a gadget.
  * GadgetView embeds a WebKit browser for this purpose.
  *
  * GadgetView exposes an instance of GadgetAPI to the gadget's javascript code.
  */
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
    GadgetAPI* gadgetAPI() const { return m_api; }

    void adjustHeight( int height );

    /**
      * Called by GadgetAPI.
      */
    void onSubmit(const QString& key, const QString& value );
    /**
      * Called by GadgetAPI.
      */
    void onSubmit(const QHash<QString,QString>& delta );

signals:
    void sizeChangeRequired(GadgetView* view);
    void submit(GadgetView* view, const QString& key, const QString& value );
    void submit(GadgetView* view, const QHash<QString,QString>& delta );

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
