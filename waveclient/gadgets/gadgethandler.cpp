#include "gadgethandler.h"
#include "gadgetview.h"
#include "view/graphicstextitem.h"
#include "view/otadapter.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
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

GadgetView* GadgetHandler::insertGadget(QTextCursor* cursor, const QUrl& url, const QString& id)
{
    GadgetView* view = new GadgetView(m_textItem->adapter()->blip(), url, m_textItem->textWidth(), id, m_environment);
    bool check = connect( view, SIGNAL(sizeChangeRequired(GadgetView*)), SLOT(resizeGadget(GadgetView*)));
    Q_ASSERT(check);
    check = m_textItem->adapter()->connect( view, SIGNAL(submit(GadgetView*,QHash<QString,QString>)), SLOT(gadgetSubmit(GadgetView*,QHash<QString,QString>)) );
    Q_ASSERT(check);
    check = m_textItem->adapter()->connect( view, SIGNAL(submit(GadgetView*,QString,QString)), SLOT(gadgetSubmit(GadgetView*,QString,QString)));
    Q_ASSERT(check);

    QGraphicsProxyWidget* item = m_textItem->scene()->addWidget(view);
    item->setParentItem( m_textItem );
    m_gadgets[id] = view;
    m_gadgetItems[id] = item;

    QTextCharFormat charFormat;
    charFormat.setObjectType(GadgetFormat);
    charFormat.setProperty(Id, id);
    cursor->insertText(QString(QChar::ObjectReplacementCharacter), charFormat);

    return view;
}

void GadgetHandler::setGadgetWidth( qreal width )
{
    foreach( QGraphicsProxyWidget* view, m_gadgetItems.values() )
    {
        QSizeF s = ((GadgetView*)view->widget())->preferredSize();
        view->setGeometry( QRectF( view->x(), view->y(), qMin( s.width(), width ), view->size().height() ) );
    }
}

void GadgetHandler::resizeGadget( GadgetView* view )
{
    QGraphicsProxyWidget* item = m_gadgetItems[ view->id() ];
    QSizeF s = view->preferredSize();
    item->setGeometry( QRectF( item->x(), item->y(), qMin( s.width(), m_textItem->textWidth() ), s.height() ) );

    // Trigger a relayout
    QTextCursor cursor = findGadget( view->id() );
    if ( !cursor.isNull() )
    {
        m_textItem->adapter()->suspendContentsChange( true );
        cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor, 1 );
        cursor.mergeCharFormat( cursor.charFormat() );
        m_textItem->adapter()->suspendContentsChange( false );
    }
}

GadgetView* GadgetHandler::gadget(const QString& id) const
{
    return m_gadgets[id];
}

QList<GadgetView*> GadgetHandler::gadgets() const
{
    QList<GadgetView*> result;
    foreach( GadgetView* view, m_gadgets.values() )
    {
        result.append(view);
    }
    return result;
}

QTextCursor GadgetHandler::findGadget(const QString& id)
{
    QTextCursor cursor( m_textItem->document() );
    cursor.movePosition( QTextCursor::Right );
    while( true )
    {
        QTextCharFormat f = cursor.charFormat();
        if ( f.objectType() == GadgetFormat )
        {
            if ( id == f.property(Id).toString() )
                return cursor;
        }
        if ( cursor.atEnd() )
            return QTextCursor();
        cursor.movePosition( QTextCursor::Right );
    }
    return QTextCursor();
}
