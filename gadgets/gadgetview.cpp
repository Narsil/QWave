#include "gadgetview.h"
#include <QUrl>

GadgetView::GadgetView(const QUrl& url)
{
    this->setGeometry( 0, 0, 300, 150 );
    this->setHtml("<html><body style=\"background-color:#888888\"><h1>Loading gadget ...</h1></body></html>");
}
