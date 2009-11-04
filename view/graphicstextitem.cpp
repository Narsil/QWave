#include "graphicstextitem.h"
#include "blipgraphicsitem.h"
#include "otadapter.h"
#include "caret.h"

#include <QTextDocument>
#include <QTextCursor>

GraphicsTextItem::GraphicsTextItem(OTAdapter* adapter, QGraphicsItem* parent)
        : QGraphicsTextItem(parent), m_forbiddenTextRange(0), m_adapter(adapter)
{
    connect(document(), SIGNAL(contentsChange(int,int,int)), SLOT(onContentsChange(int,int,int)));

    m_caretIface = CaretInterface::initialize(this->document(), this);
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
