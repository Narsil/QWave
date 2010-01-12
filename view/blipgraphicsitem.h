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

/**
  * Displayes a blip and allows the blipto be edited.
  * For details on editing see GraphicsTextItem.
  */
class BlipGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    BlipGraphicsItem(Blip* blip, qreal x, qreal y, qreal width, QGraphicsItem* parent = 0);

    QTextDocument* document();
    Blip* blip() const { return m_blip; }
    GraphicsTextItem* textItem() const { return m_text; }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    /**
      * Changes the size of the blip graphics.
      *
      * This method is called for example when the containing window (i.e. QGraphicsView) is resized.
      */
    void setWidth(qreal width);

    /**
      * Sets the icon shown in the top left corner of the blip.
      * This is meant to be the image of the blip owner.
      *
      * Called from OTAdapter.
      */
    void setAuthorPixmap(const QPixmap& pixmap);

    /**
      * Called from the GUI.
      */
    void toggleBold();
    /**
      * Called from the GUI.
      */
    void toggleItalic();
    /**
      * Called from the GUI.
      */
    void toggleUnderline();
    /**
      * Called from the GUI.
      */
    void toggleStrikeout();
    /**
      * Called from the GUI.
      */
    void insertImage( const QUrl& url, const QImage& image, const QImage& thumbnail, const QString& caption );
    /**
      * Called from the GUI.
      */
    void insertGadget( const QUrl& url );

signals:
    /**
      * Called from GraphicsTextItem when it receives keyboard focus, i.e. this
      * signal is simply passed through.
      */
    void focusIn();
    /**
      * Emitted when the size of the blip graphics changes.
      */
    void sizeChanged();
    /**
      * Emitted if this blip is the first root blip and if the first line
      * of this blip has changed.
      *
      * This signal is created inside OTAdapter and then simply passed through.
      */
    void titleChanged(const QString& title);

protected:
    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    virtual void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    /**
      * Called from QTextDocument is the text changes.
      */
    void onContentsChanged();
    /**
      * Called from the Blip if, for example, the read-state changes and if this requires a visual update, i.e. repaint.
      * Internally, this slot simply invokes update().
      */
    void repaint();

private:
    /**
      * The Blip displayed here.
      */
    Blip* m_blip;
    /**
      * The text editor
      */
    GraphicsTextItem* m_text;
    /**
      * Allows the user to create a reply blip.
      */
    BlipReplyGraphicsItem* m_replyItem;
    /**
      * Pixmap of the user who created this blip.
      */
    QPixmap m_userPixmap;
    /**
      * Last known size of the text.
      * When the text changes this is used to determine if the height of the text has changed.
      */
    QRectF m_lastTextRect;
    /**
      * Last width of the entire blip.
      * When a new width is set (setWidth) this value is used to check whether any updates are required.
      */
    qreal m_lastWidth;
    /**
      * Connects this graphics item with the OT mechanisms.
      */
    OTAdapter* m_adapter;
};

/**
  * Creates a visual element which allows users to create a reply to some blip.
  */
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
