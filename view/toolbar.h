#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QGraphicsView>

class ToolBar : public QGraphicsView
{
public:
    ToolBar(QWidget* parent = 0);

    QGraphicsScene* scene() const { return m_scene; }

    void addItem( QGraphicsItem* item );

protected:
    virtual void drawBackground( QPainter* painter, const QRectF& rect );

private:
    QGraphicsScene* m_scene;
    int m_usedWidth;
};

#endif // TOOLBAR_H
