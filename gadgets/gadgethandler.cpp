#include "gadgethandler.h"
#include "gadgetview.h"
#include "view/graphicstextitem.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QUuid>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

GadgetHandler::GadgetHandler(GraphicsTextItem* parent, Environment* environment)
        : QObject(parent), m_textItem(parent), m_environment(environment)
{
}

QSizeF GadgetHandler::intrinsicSize(QTextDocument *, int,
                                     const QTextFormat &format)
 {
     QString id = format.property(Id).toString();
     GadgetView* view = m_gadgets[id];
     if ( view )
         return QSizeF( view->width(), view->height() );
     return QSizeF(1,1);
 }

void GadgetHandler::drawObject(QPainter*, const QRectF &rect, QTextDocument*, int, const QTextFormat &format)
 {
    QString id = format.property(Id).toString();
    QGraphicsItem* item = m_gadgetItems[id];
    if ( item )
        item->setPos( rect.x(), rect.y() );
 }

GadgetHandler* GadgetHandler::initialize(QTextDocument* doc, Environment* environment, GraphicsTextItem* parent)
{
    GadgetHandler* iface = new GadgetHandler(parent, environment);
    doc->documentLayout()->registerHandler(GadgetFormat, iface);
    return iface;
}

void GadgetHandler::insertGadget(QTextCursor* cursor, const QUrl& url)
{
    QString id = QUuid::createUuid().toString();
    GadgetView* view = new GadgetView(url, m_textItem->textWidth(), m_environment);
    QGraphicsItem* item = m_textItem->scene()->addWidget(view);
    item->setParentItem( m_textItem );
    m_gadgets[id] = view;
    m_gadgetItems[id] = item;

    QTextCharFormat charFormat;
    charFormat.setObjectType(GadgetFormat);
    charFormat.setProperty(Id, id);
    cursor->insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
}
