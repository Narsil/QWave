#ifndef GADGETVIEW_H
#define GADGETVIEW_H

#include <QWebView>

class QUrl;

class GadgetView : public QWebView
{
public:
    GadgetView(const QUrl& url);
};

#endif // GADGETVIEW_H
