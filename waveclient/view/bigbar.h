#ifndef BIGBAR_H
#define BIGBAR_H

#include <QGraphicsView>

class QGraphicsScene;

class BigBar : public QGraphicsView
{
public:
    BigBar(QWidget* parent = 0);

    QGraphicsScene* scene() const { return m_scene; }

protected:
    virtual void drawBackground( QPainter* painter, const QRectF& rect );

private:
    QGraphicsScene* m_scene;
};

#endif // BIGBAR_H
