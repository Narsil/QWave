#include "imagehandler.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QColor>

ImageHandler::ImageHandler(QObject* parent)
        : QObject(parent), m_font("Arial", 8)
{
    m_leftPixmap = QPixmap("images/image_leftborder.png");
    m_rightPixmap = QPixmap("images/image_rightborder.png");
    m_topPixmap = QPixmap("images/image_topborder.png");
    m_bottomPixmap = QPixmap("images/image_bottomborder.png");
}

QSizeF ImageHandler::intrinsicSize(QTextDocument *, int,
                                     const QTextFormat &format)
 {
     return format.property(Size).toSizeF();
 }

void ImageHandler::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *, int, const QTextFormat &format)
 {
    QString id = format.property(Id).toString();
    painter->drawPixmap( rect.x(), rect.y(), m_pixmaps[id] );
 }

ImageHandler* ImageHandler::initialize(QTextDocument* doc, QObject* parent)
{
    ImageHandler* iface = new ImageHandler(parent);
    doc->documentLayout()->registerHandler(ImageFormat, iface);
    return iface;
}

void ImageHandler::insertImage(QTextCursor* cursor, const QString& id, const QImage& image, const QString& caption)
{
    QFontMetrics metrics(m_font);
    QSizeF s = metrics.size(Qt::TextSingleLine, caption);
    QSize size( (int)(6 + image.width() + 15),  qMin( 90, (int)(6 + image.height() + s.height())));

    QPixmap pixmap(size);
    pixmap.fill( Qt::white);
    {
        QPainter painter( &pixmap );
        painter.drawPixmap( 0, 0, m_leftPixmap, 0, 0, 6, size.height() );
        for( int i = 0; i < image.width(); i += 2 )
            painter.drawPixmap( 6 + i, 0, m_topPixmap );
        painter.drawPixmap( 6 + image.width(), 0, m_rightPixmap, 0, 0, 15, size.height() );
        painter.drawImage( 6, 6, image );
        painter.setFont(m_font);
        painter.drawText( 6 + (image.width() - s.width()) / 2, size.height() - 4, caption );
        painter.setPen( QPen( QColor(221,221,221)) );
        painter.drawLine( 0, size.height() - 1, 6 + image.width() + 6, size.height() - 1);
    }

     m_pixmaps[id] = pixmap;
     QTextCharFormat charFormat;
     charFormat.setObjectType(ImageFormat);
     charFormat.setProperty(Caption, caption);
     charFormat.setProperty(Size, QSizeF(size.width(), size.height()));
     charFormat.setProperty(Id, id);
     cursor->insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
}
