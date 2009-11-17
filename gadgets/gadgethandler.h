#ifndef GADGETHANDLER_H
#define GADGETHANDLER_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>
#include <QString>

class GadgetView;
class GraphicsTextItem;
class QGraphicsItem;

class GadgetHandler : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum
    {
        GadgetFormat = QTextFormat::UserObject + 3
    };
    enum GadgetData
    {
        Id = 1,
        Size = 2
    };

    GadgetHandler(GraphicsTextItem* parent);

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

    void insertGadget(QTextCursor* cursor, const QUrl& url);

    static GadgetHandler* initialize(QTextDocument* doc, GraphicsTextItem* parent);

private:
    QFont m_font;
    QHash<QString,GadgetView*> m_gadgets;
    QHash<QString,QGraphicsItem*> m_gadgetItems;
    GraphicsTextItem* m_textItem;
};

#endif // GADGETHANDLER_H
