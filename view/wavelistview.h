#ifndef WAVELISTVIEW_H
#define WAVELISTVIEW_H

#include <QGraphicsView>
#include <QHash>
#include <QString>

class WaveList;
class Wave;
class WaveDigestGraphicsItem;
class QGraphicsScene;

class WaveListView : public QGraphicsView
{
    Q_OBJECT
public:
    WaveListView(WaveList* lst, QWidget* parent = 0);

    void select( Wave* wave );

protected:
    virtual void resizeEvent( QResizeEvent* event );

private slots:
    void addWave( Wave* wave );
    void removeWave( Wave* wave );
    void itemClicked(WaveDigestGraphicsItem* item);

private:
    void layout();

    WaveList* m_list;
    QHash<QString, WaveDigestGraphicsItem*> m_items;
    WaveDigestGraphicsItem* m_selectedItem;
    QGraphicsScene* m_scene;
};

#endif // WAVELISTVIEW_H
