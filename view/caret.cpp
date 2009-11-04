#include "caret.h"
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QColor>

CaretInterface::CaretInterface(QObject* parent)
        : QObject(parent), m_font("Arial", 9)
{
    m_font.setBold(true);
}

QSizeF CaretInterface::intrinsicSize(QTextDocument *, int,
                                     const QTextFormat &format)
 {
     return format.property(Size).toSizeF();
 }

void CaretInterface::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *, int, const QTextFormat &format)
 {         
     painter->setRenderHint(QPainter::Antialiasing, true);
     int c = format.property(Color).toInt();     
     QColor color(c & 0xff, (c & 0xff00) / 0x100, (c & 0xff0000) / 0x10000);
     QString text = format.property(Text).toString();
     painter->setFont(m_font);
     // painter->fillRect(rect, QBrush(color));
     painter->setBrush(QBrush(color));
     painter->setPen(QPen(Qt::NoPen));
     QRectF r2( rect.x(), rect.y() + 2, rect.width(), rect.height() + 2);
     painter->drawRoundedRect(r2, 3, 3);
     painter->setPen(QPen(Qt::white));
     QRectF r( rect.x() + 2, rect.y() + 3, rect.width() - 4, rect.height() - 2);
     painter->drawText(r, text);
     painter->setRenderHint(QPainter::Antialiasing, false);
 }

void CaretInterface::insertCaret(QTextCursor* cursor, const QString& text, const QColor& color, const QString& owner)
 {
     QTextCharFormat charFormat;
     charFormat.setObjectType(CaretFormat);
     charFormat.setProperty(Text, text);
     charFormat.setProperty(Owner, owner);
     charFormat.setProperty(Size, textSize(text));
     charFormat.setProperty(Color, color.red() + color.green() * 0x100 + color.blue() * 0x10000);
     cursor->insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
 }

QSizeF CaretInterface::textSize(const QString& text)
{
    QFontMetrics metrics(m_font);
    QSizeF s = metrics.size(Qt::TextSingleLine, text);
    s.setWidth( s.width() + 4 );
    s.setHeight( s.height() + 2 );
    return s;
}

CaretInterface* CaretInterface::initialize(QTextDocument* doc, QObject* parent)
{
    CaretInterface* iface = new CaretInterface(parent);
    doc->documentLayout()->registerHandler(CaretFormat, iface);
    return iface;
}

QString CaretInterface::caretText(const QTextCursor& cursor)
{
    QTextCharFormat fmt = cursor.charFormat();
    if ( fmt.objectType() == CaretFormat )
        return fmt.property(Text).toString();
    return QString::null;
}

QString CaretInterface::caretOwner(const QTextCursor& cursor)
{
    QTextCharFormat fmt = cursor.charFormat();
    if ( fmt.objectType() == CaretFormat )
        return fmt.property(Owner).toString();
    return QString::null;
}
