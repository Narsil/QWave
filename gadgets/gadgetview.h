#ifndef GADGETVIEW_H
#define GADGETVIEW_H

#include <QWebView>

class GadgetAPI;
class GadgetManifest;
class Environment;
class QUrl;

class GadgetView : public QWebView
{
    Q_OBJECT
public:
    GadgetView(const QUrl& url, int width, Environment* environment);

private slots:
    void manifestFinished();

private:
    GadgetAPI* m_api;
    GadgetManifest* m_manifest;
    Environment* m_environment;
};

#endif // GADGETVIEW_H
