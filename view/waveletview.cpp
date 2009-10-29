#include "waveletview.h"
#include "waveview.h"
#include "model/wavelet.h"
#include "waveletgraphicsitem.h"
#include "blipgraphicsitem.h"
#include "model/blip.h"
#include "model/blipthread.h"

#include <QGraphicsScene>

WaveletView::WaveletView( WaveView* parent, Wavelet* wavelet )
        : QObject( parent ), m_wavelet(wavelet)
{
    m_gfx = new WaveletGraphicsItem(this);
    m_gfx->setPos(0,0);
    m_gfx->setZValue(1);
    headScene()->addItem(m_gfx);

    layoutBlips(100);

    connect( wavelet, SIGNAL(conversationChanged()), SLOT(layoutBlips()));
}

WaveletView::~WaveletView()
{
    delete m_gfx;
}

void WaveletView::setWavelet( Wavelet* wavelet )
{
    if ( m_wavelet )
        disconnect(m_wavelet, SIGNAL(conversationChanged()), this, SLOT(layoutBlips()));
    m_wavelet = wavelet;
    connect( wavelet, SIGNAL(conversationChanged()), this, SLOT(layoutBlips()));
    m_gfx->setWavelet(wavelet);
    foreach( BlipGraphicsItem* item, m_blipItems.values() )
    {
        delete item;
    }
    m_blipItems.clear();
    layoutBlips(m_lastWidth);
}

void WaveletView::layoutBlips()
{
    layoutBlips(m_lastWidth);
}

void WaveletView::layoutBlips(qreal width)
{
    m_lastWidth = width;
    qreal dy = 0;
    qreal dx = 0;
    foreach( Blip* blip, wavelet()->rootBlips() )
    {
        layoutBlip(blip, dx, dy, width);
    }
}

void WaveletView::layoutBlip(Blip* blip, qreal& xoffset, qreal& yoffset, qreal width )
{
    bool created = false;
    BlipGraphicsItem* b = m_blipItems[blip->id()];
    if ( !b )
    {
        b = new BlipGraphicsItem( this, blip, width - xoffset );
        m_blipItems[blip->id()] = b;
        created = true;
    }
    else
        b->setWidth(width - xoffset);
    // b->setZValue( m_gfx->zValue() + 1 );
    // scene()->addItem(b);
    b->setPos( xoffset, yoffset );
    if ( created )
        scene()->addItem(b);
    yoffset += b->boundingRect().height();

    qreal dx = xoffset + 42;
    foreach( BlipThread* thread, blip->threads() )
    {
        foreach( Blip* b2, thread->blips() )
        {
            layoutBlip(b2, dx, yoffset, width);
        }
    }
}

QGraphicsScene* WaveletView::scene()
{
    return ((WaveView*)parent())->scene();
}

QGraphicsScene* WaveletView::headScene()
{
    return ((WaveView*)parent())->headScene();
}

void WaveletView::fitToWidth( qreal headWidth, qreal width )
{
    m_gfx->setWidth(headWidth);
    layoutBlips(width);
}

void WaveletView::setTitle( const QString& title )
{
    if ( title.isEmpty() )
        m_gfx->setTitle("(no title)");
    else
        m_gfx->setTitle(title);
}
