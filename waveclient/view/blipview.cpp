#include "blipview.h"
#include "waveletview.h"
#include "blipgraphicsitem.h"

#include <QGraphicsScene>

BlipView::BlipView( WaveletView* parent, Blip* blip )
        : QObject( parent ), m_blip(blip)
{
    //m_gfx = new BlipGraphicsItem(this);
    scene()->addItem(m_gfx);
}

BlipView::~BlipView()
{
    delete m_gfx;
}

QGraphicsScene* BlipView::scene()
{
    return ((WaveletView*)parent())->scene();
}
