#include "gadgetview.h"
#include "gadgetmanifest.h"
#include "gadgetapi.h"
#include <QUrl>

GadgetView::GadgetView(const QUrl& url, int width, Environment* environment)
        : QWebView(), m_environment(environment)
{
    this->setGeometry( 0, 0, width, 150 );
    this->setHtml("<html><body style=\"background-color:#888888; overflow:hidden\"><h1>Loading gadget ...</h1></body></html>");

    m_api = new GadgetAPI( page()->mainFrame(), this );
    m_manifest = new GadgetManifest( url, m_environment, this );
    connect( m_manifest, SIGNAL(finished()), SLOT(manifestFinished()));
    m_manifest->load();
}

void GadgetView::manifestFinished()
{
    QString content = m_manifest->content();
    this->setHtml("<html><body style=\"overflow:hidden\">" + content + "</body></html>");
}
