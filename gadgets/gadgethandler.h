#ifndef GADGETHANDLER_H
#define GADGETHANDLER_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>
#include <QTextCursor>
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

    GadgetView* insertGadget(QTextCursor* cursor, const QUrl& url, const QString& id);
    /**
      * Called when the containing text document is resized.
      */
    void setGadgetWidth( qreal width );
    QTextCursor findGadget(const QString& id);

    GadgetView* gadget(const QString& id) const;

    static GadgetHandler* initialize(QTextDocument* doc, Environment* environment, GraphicsTextItem* parent);

private slots:
    /**
      * Called when a gadget requests a new size.
      */
    void resizeGadget( GadgetView* view );

private:
    GadgetHandler(GraphicsTextItem* parent, Environment* environment);

    QFont m_font;
    QHash<QString,GadgetView*> m_gadgets;
    QHash<QString,QGraphicsProxyWidget*> m_gadgetItems;
    GraphicsTextItem* m_textItem;
    Environment* m_environment;
};

#endif // GADGETHANDLER_H
