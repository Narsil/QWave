#include "gadgethandler.h"
#include "gadgetview.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QUuid>

GadgetHandler::GadgetHandler(QObject* parent)
        : QObject(parent)
{
}

QSizeF GadgetHandler::intrinsicSize(QTextDocument *, int,
                                     const QTextFormat &format)
 {
     return format.property(Size).toSizeF();
 }

void GadgetHandler::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *, int, const QTextFormat &format)
 {
    // TODO
 }

GadgetHandler* GadgetHandler::initialize(QTextDocument* doc, QObject* parent)
{
    GadgetHandler* iface = new GadgetHandler(parent);
    doc->documentLayout()->registerHandler(GadgetFormat, iface);
    return iface;
}

void GadgetHandler::insertGadget(QTextCursor* cursor, const QUrl& url)
{
    QString id = QUuid::createUuid().toString();

    QTextCharFormat charFormat;
    charFormat.setObjectType(GadgetFormat);
    // TODO
    // charFormat.setProperty(Size, QSizeF(size.width(), size.height()));
    charFormat.setProperty(Id, id);
    cursor->insertText(QString(QChar::ObjectReplacementCharacter), charFormat);
}
