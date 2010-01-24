#include "graphicstextitem.h"
#include "blipgraphicsitem.h"
#include "otadapter.h"
#include "caret.h"
#include "imagehandler.h"
#include "gadgets/gadgethandler.h"
#include "model/blip.h"
#include "model/wavelet.h"

#include <QTextDocument>
#include <QTextCursor>

GraphicsTextItem::GraphicsTextItem(Blip* blip, QGraphicsItem* parent)
        : QGraphicsTextItem(parent), m_forbiddenTextRange(0), m_blip(blip)
{
    setTextInteractionFlags( Qt::TextEditorInteraction);

    m_adapter = new OTAdapter(this);

    // Mark the type
    setData( 1, 1 );

    // Undo is not supported by the Wave protocol currently.
    document()->setUndoRedoEnabled(false);

    connect(document(), SIGNAL(contentsChange(int,int,int)), SLOT(onContentsChange(int,int,int)));

    m_caretIface = CaretInterface::initialize(this->document(), this);
    m_imageHandler = ImageHandler::initialize(this->document(), this);
    m_gadgetHandler = GadgetHandler::initialize(this->document(), m_adapter->environment(), this);
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

void GraphicsTextItem::insertImage( const QUrl& url, const QImage& image, const QImage& thumbnail, const QString& caption )
{
    // Create an attachment
    QString attachmentId = m_adapter->blip()->wavelet()->insertImageAttachment( url, image, thumbnail );
    // Insert the image tag
    int index = m_adapter->mapToBlip( textCursor().position() );
    m_adapter->blip()->insertImage( index, attachmentId, caption );
}

void GraphicsTextItem::insertImage( QTextCursor* cursor, const QString& id, const QImage& image, const QString& caption )
{
    m_imageHandler->insertImage( cursor, id, image, caption );
}

void GraphicsTextItem::insertGadget( const QUrl& url )
{
    // Insert the gadget tag
    int index = m_adapter->mapToBlip( textCursor().position() );
    m_adapter->blip()->insertGadget( index, url );
}

GadgetView* GraphicsTextItem::insertGadget( QTextCursor* cursor, const QUrl& url, const QString& id )
{
    return m_gadgetHandler->insertGadget( cursor, url, id );
}

GadgetView* GraphicsTextItem::gadget( const QString& id ) const
{
    return m_gadgetHandler->gadget(id);
}

QList<GadgetView*> GraphicsTextItem::gadgets() const
{
    return m_gadgetHandler->gadgets();
}

void GraphicsTextItem::focusInEvent( QFocusEvent* event )
{
    QGraphicsTextItem::focusInEvent(event);
    emit focusIn();
}

void GraphicsTextItem::updateWidth( qreal width )
{
    m_gadgetHandler->setGadgetWidth(width);
    setTextWidth( width );
}

void GraphicsTextItem::toggleBold()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    QString value;
    if ( format.fontWeight() == QFont::Normal )
    {
        format.setFontWeight( QFont::Bold );
        value = "bold";
    }
    else
    {
        format.setFontWeight( QFont::Normal );
        value = QString::null;
    }

    // Tell the wave server that something has been formatted (if there is a selection).
    if ( cursor.selectionEnd() != cursor.selectionStart() )
    {
        m_adapter->onStyleChange( cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(), "style/fontWeight", value );
    }

    // Change the format for the cursor or the selection.
    m_adapter->suspendContentsChange(true);
    cursor.mergeCharFormat( format );
    m_adapter->suspendContentsChange(false);
}

void GraphicsTextItem::toggleItalic()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    QString value;
    if ( !format.fontItalic() )
    {
        format.setFontItalic(true);
        value = "italic";
    }
    else
    {
        format.setFontItalic( false );
        value = QString::null;
    }

    // Tell the wave server that something has been formatted (if there is a selection).
    if ( cursor.selectionEnd() != cursor.selectionStart() )
    {
        m_adapter->onStyleChange( cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(), "style/fontStyle", value );
    }

    // Change the format for the cursor or the selection.
    m_adapter->suspendContentsChange(true);
    cursor.mergeCharFormat( format );
    m_adapter->suspendContentsChange(false);
}

void GraphicsTextItem::toggleUnderline()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    QString value;
    if ( !format.fontUnderline() )
    {
        format.setFontUnderline(true);
        format.setFontStrikeOut(false);
        value = "underline";
    }
    else
    {
        format.setFontUnderline( false );
        value = QString::null;
    }

    // Tell the wave server that something has been formatted (if there is a selection).
    if ( cursor.selectionEnd() != cursor.selectionStart() )
    {
        m_adapter->onStyleChange( cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(), "style/textDecoration", value );
    }

    // Change the format for the cursor or the selection.
    m_adapter->suspendContentsChange(true);
    cursor.mergeCharFormat( format );
    m_adapter->suspendContentsChange(false);
}

void GraphicsTextItem::toggleStrikeout()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = textCursor();
    QTextCharFormat format = cursor.charFormat();
    QString value;
    if ( !format.fontStrikeOut() )
    {
        format.setFontStrikeOut(true);
        format.setFontUnderline(false);
        value = "line-through";
    }
    else
    {
        format.setFontStrikeOut( false );
        value = QString::null;
    }

    // Tell the wave server that something has been formatted (if there is a selection).
    if ( cursor.selectionEnd() != cursor.selectionStart() )
    {
        m_adapter->onStyleChange( cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart(), "style/textDecoration", value );
    }

    // Change the format for the cursor or the selection.
    m_adapter->suspendContentsChange(true);
    cursor.mergeCharFormat( format );
    m_adapter->suspendContentsChange(false);
}
