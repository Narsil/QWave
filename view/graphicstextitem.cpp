#include "graphicstextitem.h"
#include "blipgraphicsitem.h"
#include "otadapter.h"
#include "caret.h"
#include "imagehandler.h"
#include "gadgets/gadgethandler.h"

#include <QTextDocument>
#include <QTextCursor>

GraphicsTextItem::GraphicsTextItem(OTAdapter* adapter, QGraphicsItem* parent)
        : QGraphicsTextItem(parent), m_forbiddenTextRange(0), m_adapter(adapter)
{
    // Mark the type
    setData( 1, 1 );

    // Undo is not supported by the Wave protocol currently.
    document()->setUndoRedoEnabled(false);

    connect(document(), SIGNAL(contentsChange(int,int,int)), SLOT(onContentsChange(int,int,int)));

    m_caretIface = CaretInterface::initialize(this->document(), this);
    m_imageHandler = ImageHandler::initialize(this->document(), this);
}

GraphicsTextItem* GraphicsTextItem::cast( QGraphicsItem* item )
{
    if ( item && item->data(1).toInt() == 1 )
        return static_cast<GraphicsTextItem*>(item);
    return 0;
}

void GraphicsTextItem::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    QGraphicsTextItem::mousePressEvent(event);
    checkCursor();
}

void GraphicsTextItem::keyPressEvent( QKeyEvent * event )
{
    QGraphicsTextItem::keyPressEvent(event);
    checkCursor();
}

void GraphicsTextItem::checkCursor()
{
    QTextCursor c( textCursor() );
    int pos = c.position();
    if ( pos < m_forbiddenTextRange )
    {
        c.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_forbiddenTextRange - c.position());
        setTextCursor(c);
    }
}

void GraphicsTextItem::onContentsChange( int position, int charsRemoved, int charsAdded )
{
    m_adapter->onContentsChange( position, charsRemoved, charsAdded );
}

void GraphicsTextItem::insertCaret( QTextCursor* cursor, const QString& text, const QColor& color, const QString& owner )
{
    m_caretIface->insertCaret(cursor, text, color, owner);
}

void GraphicsTextItem::insertImage( QTextCursor* cursor, const QString& id, const QImage& image, const QString& caption )
{
    m_imageHandler->insertImage( cursor, id, image, caption );
}

void GraphicsTextItem::insertGadget( QTextCursor* cursor, const QUrl& url )
{
    m_gadgetHandler->insertGadget( cursor, url );
}

void GraphicsTextItem::focusInEvent( QFocusEvent* event )
{
    QGraphicsTextItem::focusInEvent(event);
    emit focusIn();
}
