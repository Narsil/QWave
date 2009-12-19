#ifndef GRAPHICSTEXTITEM_H
#define GRAPHICSTEXTITEM_H

#include <QGraphicsTextItem>

class OTAdapter;
class CaretInterface;
class ImageHandler;
class GadgetHandler;
class GadgetView;
class QImage;
class QUrl;

/**
  * Implementation of the core text editor component used for editing a blip.
  */
class GraphicsTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    GraphicsTextItem(OTAdapter* adapter, QGraphicsItem* parent = 0);

    void setForbiddenTextRange( int length ) { m_forbiddenTextRange = length; }
    int forbiddenTextRange() const { return m_forbiddenTextRange; }

    void insertCaret( QTextCursor* cursor, const QString& text, const QColor& color, const QString& owner );
    /**
      * Called from the GUI or from OTAdapter when applying OT deltas.
      */
    void insertImage( QTextCursor* cursor, const QString& id, const QImage& image, const QString& caption );
    /**
      * Called from the GUI or from OTAdapter when applying OT deltas.
      */
    GadgetView* insertGadget( QTextCursor* cursor, const QUrl& url, const QString& id );

    /**
      * Call this function instead of setTextWidth, because this function will as well
      * resize the gadgets.
      */
    void updateWidth( qreal width );

    OTAdapter* adapter() const { return m_adapter; }

    GadgetView* gadget( const QString& id ) const;

    static GraphicsTextItem* cast( QGraphicsItem* item );

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void focusInEvent( QFocusEvent* event );

signals:
    void focusIn();

private slots:
    void onContentsChange( int position, int charsRemoved, int charsAdded );

private:
    void checkCursor();

    int m_forbiddenTextRange;
    OTAdapter* m_adapter;
    CaretInterface* m_caretIface;
    ImageHandler* m_imageHandler;
    GadgetHandler* m_gadgetHandler;
};

#endif // GRAPHICSTEXTITEM_H
