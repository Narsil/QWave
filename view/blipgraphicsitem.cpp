#include "blipgraphicsitem.h"
#include "model/blip.h"
#include "caret.h"
#include "model/participant.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "app/environment.h"
#include "graphicstextitem.h"
#include "otadapter.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTextDocument>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainter>
#include <QPixmap>
#include <QTextBlock>
#include <QCursor>
#include <QTextCursor>
#include <QImage>
#include <QUrl>

BlipGraphicsItem::BlipGraphicsItem(Blip* blip, qreal x, qreal y, qreal width, QGraphicsItem* parent)
        : QObject(), QGraphicsItem(parent), m_blip(blip), m_replyItem( 0 ), m_lastWidth(width)
{
    setAcceptHoverEvents(true);
    setPos( x, y );

    m_adapter = new OTAdapter(this);

    m_text = new GraphicsTextItem(m_adapter, this);
    m_text->setTextInteractionFlags( Qt::TextEditorInteraction);
    if ( m_blip->isRootBlip() )
        m_text->setPos(40,2);
    else
        m_text->setPos(44,2);
    m_text->setTextWidth(width - m_text->x());
    connect( m_text, SIGNAL(focusIn()), SIGNAL(focusIn()));

    QObject::connect(m_adapter, SIGNAL(titleChanged(const QString&)), SIGNAL(titleChanged(const QString&)));
    // Show the contents of the document
    m_adapter->setGraphicsText();

    m_lastTextRect = m_text->boundingRect();
    QObject::connect(m_text->document(), SIGNAL(contentsChanged()), SLOT(onContentsChanged()));

    connect( blip, SIGNAL(unreadChanged()), SLOT(repaint()));
}

QTextDocument* BlipGraphicsItem::document()
{
    return m_text->document();
}

void BlipGraphicsItem::setWidth(qreal width)
{
    if ( m_lastWidth == width )
        return;
    m_lastWidth = width;
    m_text->updateWidth(width - m_text->x());
}

void BlipGraphicsItem::repaint()
{
    update();
}

void BlipGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF rect = boundingRect();

    if ( m_blip->isRootBlip() )
        painter->drawPixmap(12, 6, m_userPixmap);
    else
        painter->drawPixmap(16, 6, m_userPixmap);

    if ( m_blip->isLastBlipInThread() && !m_blip->isRootBlip() )
    {
        QLinearGradient grad2(QPoint(0,0), QPoint(6,0));
        grad2.setColorAt(1, QColor(220, 220, 220));
        grad2.setColorAt(0, QColor(255, 255, 255));
        QBrush brush2( grad2 );
        painter->fillRect(0, 0, 6, rect.height() - 6, brush2);
        QPen pen(QColor(180,180,180));
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawLine(6, 0, 6, rect.height() - 7);

        QLinearGradient grad(QPoint(0,rect.height() - 6), QPoint(0,rect.height()));
        grad.setColorAt(0, QColor(220, 220, 220));
        grad.setColorAt(1, QColor(255, 255, 255));
        QBrush brush( grad );
        painter->fillRect(6, rect.height() - 6, rect.width() - 6, 6, brush);
        painter->drawLine(6, rect.height() - 7, rect.width(), rect.height() - 7);

        QRadialGradient grad3(6, rect.height() - 6, 6, 0, 0);
        grad3.setColorAt(0, QColor(220, 220, 220));
        grad3.setColorAt(1, QColor(255, 255, 255));
        QBrush brush3( grad3 );
        painter->fillRect(0, rect.height() - 6, 6, 6, brush3);
    }
    else if ( !m_blip->isRootBlip() )
    {
        QLinearGradient grad2(QPoint(0,0), QPoint(6,0));
        grad2.setColorAt(1, QColor(220, 220, 220));
        grad2.setColorAt(0, QColor(255, 255, 255));
        QBrush brush2( grad2 );
        painter->fillRect(0, 0, 6, rect.height(), brush2);
        QPen pen(QColor(180,180,180));
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawLine(6, 0, 6, rect.height());
    }

    if ( !m_blip->isFirstRootBlip() )
    {
        QPen pen2(QColor(180,180,180));
        pen2.setStyle(Qt::DotLine);
        painter->setPen(pen2);
        painter->drawLine(12, 0, rect.width(), 0);
    }

    if ( m_blip->isUnread() )
    {
        QBrush b(QColor(0x99,0xbb,0));
        if ( m_blip->isRootBlip() )
            painter->fillRect(4, 3, 3, 38, b);
        else
            painter->fillRect(9, 3, 3, 38, b);
    }
}

void BlipGraphicsItem::onContentsChanged()
{
    if ( m_replyItem )
    {
        delete m_replyItem;
        m_replyItem = 0;
    }

    QRectF r = m_text->boundingRect();
    if ( m_lastTextRect != r )
    {
        m_lastTextRect = r;
        prepareGeometryChange();
        // m_view->layoutBlips();
        emit sizeChanged();
    }
}

QRectF BlipGraphicsItem::boundingRect() const
{
    QRectF rect = m_text->boundingRect();
    if ( m_blip->isRootBlip() )
        return QRectF( 0, 0, rect.width() + 40, qMax( 38.0, rect.height() ) + 12 );
    return QRectF( 0, 0, rect.width() + 44, qMax( 38.0, rect.height() ) + 12 );
}

void BlipGraphicsItem::setAuthorPixmap(const QPixmap& pixmap)
{
    // scale the pixmap
    m_userPixmap = pixmap.scaledToWidth(28, Qt::SmoothTransformation);
    // draw it
    update();
}

void BlipGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
    if ( !m_replyItem && !scene()->focusItem() )
    {
        m_replyItem = new BlipReplyGraphicsItem(this);
    }
}

void BlipGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    if ( m_replyItem )
    {
        delete m_replyItem;
        m_replyItem = 0;
    }
}

void BlipGraphicsItem::hoverMoveEvent ( QGraphicsSceneHoverEvent* )
{
    if ( m_replyItem && scene()->focusItem() )
    {
        delete m_replyItem;
        m_replyItem = 0;
    }
}

//void BlipGraphicsItem::titleChanged(const QString& title)
//{
//    m_view->setTitle(title);
//}

void BlipGraphicsItem::toggleBold()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = m_text->textCursor();
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

void BlipGraphicsItem::toggleItalic()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = m_text->textCursor();
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

void BlipGraphicsItem::toggleUnderline()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = m_text->textCursor();
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

void BlipGraphicsItem::toggleStrikeout()
{
    // Find out wether to turn the style on or off
    QTextCursor cursor = m_text->textCursor();
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

//void BlipGraphicsItem::focusInEvent()
//{
//    m_view->focusInEvent( this );
//}

void BlipGraphicsItem::insertImage( const QUrl& url, const QImage& image, const QImage& thumbnail, const QString& caption )
{
    // Create an attachment
    QString attachmentId = blip()->wavelet()->insertImageAttachment( url, image, thumbnail );
    // Insert the image tag
    int index = m_adapter->mapToBlip( m_text->textCursor().position() );
    blip()->insertImage( index, attachmentId, caption );

    // Display the image in the QTextDocument
//    m_adapter->suspendContentsChange(true);
//    QTextCursor cursor( m_text->textCursor() );
//    m_text->insertImage( &cursor, attachmentId, thumbnail, caption );
//    m_adapter->suspendContentsChange(false);
}

void BlipGraphicsItem::insertGadget( const QUrl& url )
{
    // Insert the gadget tag
    int index = m_adapter->mapToBlip( m_text->textCursor().position() );
    blip()->insertGadget( index, url );

    // Display the image in the QTextDocument
//    m_adapter->suspendContentsChange(true);
//    QTextCursor cursor( m_text->textCursor() );
//    m_text->insertGadget( &cursor, url );
//    m_adapter->suspendContentsChange(false);
}

void BlipGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent* )
{
    if (m_blip->isUnread())
        m_blip->setUnread(false);
    else
        m_blip->setUnread(true);
}

/****************************************************************************
 *
 * BlipReplyGraphicsItem
 *
 ***************************************************************************/

QPixmap* BlipReplyGraphicsItem::s_replyPixmap = 0;
QPixmap* BlipReplyGraphicsItem::s_continuePixmap = 0;

BlipReplyGraphicsItem::BlipReplyGraphicsItem( BlipGraphicsItem* item )
        : QGraphicsItem(item)
{
    initialize();

    QRectF r = item->boundingRect();
    m_rect = QRectF(0, 0, r.width(), 19);
    setPos(0, r.height() - m_rect.height());
}

void BlipReplyGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    float dx = 6 + pixmap()->width() + 1;
    float dy = m_rect.height() - 6;
    QPen pen3(QColor(0,0,255));
    pen3.setWidth(2);
    painter->setPen(pen3);
    painter->drawLine(dx + 3, dy, m_rect.width() - 3, dy);
    painter->drawLine(dx + 3, m_rect.height() - 2, m_rect.width() - 3, m_rect.height() - 2);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawArc(QRectF(dx, dy, 4, 4), 90 * 16, 180 * 16);
    painter->drawArc(QRectF(m_rect.width()-4, dy, 4, 4), 270 * 16, 180 * 16);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawPixmap(6, m_rect.height() - pixmap()->height(), *pixmap());
}

QRectF BlipReplyGraphicsItem::boundingRect() const
{
    return m_rect;
}

void BlipReplyGraphicsItem::initialize()
{
    if ( s_replyPixmap == 0 )
        s_replyPixmap = new QPixmap("images/reply.png");
    if ( s_continuePixmap == 0 )
        s_continuePixmap = new QPixmap("images/continue.png");
}

QPixmap* BlipReplyGraphicsItem::pixmap()
{
    if ( ((BlipGraphicsItem*)parentItem())->blip()->isLastBlipInThread() )
        return s_continuePixmap;
    return s_replyPixmap;
}

void BlipReplyGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent* )
{
    if ( ((BlipGraphicsItem*)parentItem())->blip()->isLastBlipInThread() )
        ((BlipGraphicsItem*)parentItem())->blip()->createFollowUpBlip();
    else
        ((BlipGraphicsItem*)parentItem())->blip()->createReplyBlip();
}
