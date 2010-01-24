#include "wavelistview.h"
#include "model/wavelist.h"
#include "model/wave.h"
#include "wavedigestgraphicsitem.h"
#include "app/environment.h"

#include <QGraphicsScene>

WaveListView::WaveListView(WaveList* list, QWidget* parent)
        : QGraphicsView(parent), m_list(list), m_selectedItem(0)
{
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setLineWidth(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setAttribute(Qt::WA_Hover, true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scene = new QGraphicsScene(this);
    setScene( m_scene );
    // setSceneRect(0,0,400,400);

    connect( list, SIGNAL(waveAdded(Wave*)), SLOT(addWave(Wave*)));
    connect( list, SIGNAL(waveRemoved(Wave*)), SLOT(removeWave(Wave*)));

    foreach( Wave* w, list->waves() )
    {
        addWave(w);
    }
}

void WaveListView::addWave( Wave* wave )
{
    if ( m_items.contains(wave->id() ) )
        return;
    WaveDigestGraphicsItem* item = new WaveDigestGraphicsItem(wave, frameRect().width());
    m_items[wave->id()] = item;
    m_scene->addItem(item);
    connect( item, SIGNAL(clicked(WaveDigestGraphicsItem*)), SLOT(itemClicked(WaveDigestGraphicsItem*)));
    layout();
}

void WaveListView::removeWave( Wave* wave )
{
    if ( m_selectedItem->wave() == wave )
        m_selectedItem = 0;
    if ( !m_items.contains(wave->id() ) )
        return;    
    WaveDigestGraphicsItem* item = m_items[wave->id()];
    delete item;
    m_items.remove(wave->id() );
    layout();
}

void WaveListView::layout()
{
    int y = 0;
    foreach( WaveDigestGraphicsItem* item, m_items.values() )
    {
        item->setPos(0, y);
        y += item->boundingRect().height();
    }
    this->setSceneRect( 0, 0, frameRect().width(), y );
}

void WaveListView::resizeEvent( QResizeEvent* )
{
    foreach( WaveDigestGraphicsItem* item, m_items.values() )
    {
        item->setWidth(frameRect().width() );
    }

    this->setSceneRect( 0, 0, frameRect().width(), sceneRect().height() );
}

void WaveListView::itemClicked(WaveDigestGraphicsItem* item)
{
    if ( m_selectedItem == item )
        return;
    if ( m_selectedItem )
        m_selectedItem->setSelected(false);
    m_selectedItem = item;
    m_selectedItem->setSelected(true);

    emit selected(item->wave());
//    item->wave()->environment()->networkAdapter()->openWavelet( item->wave()->wavelet() );
}

void WaveListView::select( Wave* wave )
{
    WaveDigestGraphicsItem* item = m_items[wave->id()];
    if ( !item )
        return;
    itemClicked(item);
}

Wave* WaveListView::selectedWave()
{
	if (m_selectedItem!=0)
		return m_selectedItem->wave();
	return 0;
}
