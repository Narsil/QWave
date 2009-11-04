#ifndef GRAPHICSTEXTITEM_H
#define GRAPHICSTEXTITEM_H

#include <QGraphicsTextItem>

class OTAdapter;
class CaretInterface;

class GraphicsTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    GraphicsTextItem(OTAdapter* adapter, QGraphicsItem* parent = 0);

    void setForbiddenTextRange( int length ) { m_forbiddenTextRange = length; }
    int forbiddenTextRange() const { return m_forbiddenTextRange; }

    void insertCaret( QTextCursor* cursor, const QString& text, const QColor& color );

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );

private slots:
    void onContentsChange( int position, int charsRemoved, int charsAdded );

private:
    void checkCursor();

    int m_forbiddenTextRange;
    OTAdapter* m_adapter;
    CaretInterface* m_caretIface;
};

#endif // GRAPHICSTEXTITEM_H
