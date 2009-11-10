#ifndef BLIPGRAPHICSVIEWITEM_H
#define BLIPGRAPHICSVIEWITEM_H

#include <QGraphicsItem>
#include <QRectF>
#include <QPixmap>

class GraphicsTextItem;
class QGraphicsView;
class QTextDocument;
class QImage;
class QUrl;

class WaveletView;
class BlipReplyGraphicsItem;
class Blip;
class OTAdapter;

class BlipGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    BlipGraphicsItem(WaveletView* view, Blip* blip, qreal width);

    QTextDocument* document();
    Blip* blip() const { return m_blip; }
    GraphicsTextItem* textItem() const { return m_text; }
    WaveletView* view() const { return m_view; }    

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void setWidth(qreal width);

    void setAuthorPixmap(const QPixmap& pixmap);

    void toggleBold();
    void insertImage( const QUrl& url, const QImage& image, const QImage& thumbnail, const QString& caption );

protected:
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );

private slots:
    void onContentsChanged();
    void titleChanged(const QString& title);
    void focusInEvent();
    void repaint();

private:
    Blip* m_blip;
    GraphicsTextItem* m_text;
    BlipReplyGraphicsItem* m_replyItem;
    QPixmap m_userPixmap;
    QRectF m_lastTextRect;
    WaveletView* m_view;
    qreal m_lastWidth;
    OTAdapter* m_adapter;
};

class BlipReplyGraphicsItem : public QGraphicsItem
{
public:
    BlipReplyGraphicsItem(  BlipGraphicsItem* parent );

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private:
    QPixmap* pixmap();
    void initialize();

    QRectF m_rect;

    static QPixmap* s_replyPixmap;
    static QPixmap* s_continuePixmap;
};

#endif
