#ifndef GADGETHANDLER_H
#define GADGETHANDLER_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>
#include <QString>

class GadgetView;
class GraphicsTextItem;
class QGraphicsProxyWidget;
class Environment;

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

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

    void insertGadget(QTextCursor* cursor, const QUrl& url);
    void setGadgetWidth( qreal width );

    static GadgetHandler* initialize(QTextDocument* doc, Environment* environment, GraphicsTextItem* parent);

private:
    GadgetHandler(GraphicsTextItem* parent, Environment* environment);

    QFont m_font;
    QHash<QString,GadgetView*> m_gadgets;
    QHash<QString,QGraphicsProxyWidget*> m_gadgetItems;
    GraphicsTextItem* m_textItem;
    Environment* m_environment;
};

#endif // GADGETHANDLER_H
