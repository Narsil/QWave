#include "gadgetview.h"
#include "gadgetmanifest.h"
#include "gadgetapi.h"
#include <QUrl>

GadgetView::GadgetView(const QUrl& url, int width, const QString& id, Environment* environment)
        : QWebView(), m_environment(environment), m_preferredSize( width, 150 ), m_id(id)
{
    setGeometry( 0, 0, (int)m_preferredSize.width(), (int)m_preferredSize.height() );
    this->setHtml("<html><body style=\"background-color:#888888; overflow:hidden\"><h1>Loading gadget ...</h1></body></html>");

    m_api = new GadgetAPI( this, page()->mainFrame(), this );
    m_manifest = new GadgetManifest( url, m_environment, this );
    connect( m_manifest, SIGNAL(finished()), SLOT(manifestFinished()));
    m_manifest->load();
}

void GadgetView::manifestFinished()
{
    m_preferredSize = QSizeF( m_manifest->width(), m_manifest->height() );

    emit sizeChangeRequired(this);

    QString content = m_manifest->content();
    this->setHtml("<html><body style=\"overflow:hidden; border:0px; padding:0px; margin:0px\"><div id=\"__os__container\" style=\"position:absolute; border:0px; padding:0px; margin:0px\" >" + content + "</div></body></html>");
}

void GadgetView::adjustHeight( int height )
{
    qDebug("Adjust Height %i", height );
    height += 10; // To be on the save side, it seems that sometimes pixels are missing
    if ( m_preferredSize.height() == height )
        return;
    m_preferredSize = QSizeF( m_preferredSize.width(), height );
    emit sizeChangeRequired(this);
}
