#include "waveletview.h"
#include "waveview.h"
#include "model/wavelet.h"
#include "blipgraphicsitem.h"
#include "model/blip.h"
#include "model/blipthread.h"

#include <QGraphicsScene>

WaveletView::WaveletView( WaveView* parent, Wavelet* wavelet )
        : QGraphicsView( parent ), m_wavelet(wavelet)
{
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setLineWidth(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setAttribute(Qt::WA_Hover, true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scene = new QGraphicsScene();
    setScene( m_scene );

    layoutBlips();

    connect( wavelet, SIGNAL(conversationChanged()), SLOT(layoutBlips()));
}

WaveletView::~WaveletView()
{
}

void WaveletView::setWavelet( Wavelet* wavelet )
{
    if ( m_wavelet )
        disconnect(m_wavelet, SIGNAL(conversationChanged()), this, SLOT(layoutBlips()));
    m_wavelet = wavelet;
    connect( wavelet, SIGNAL(conversationChanged()), this, SLOT(layoutBlips()));
    foreach( BlipGraphicsItem* item, m_blipItems.values() )
    {
        delete item;
    }
    m_blipItems.clear();
    layoutBlips( frameRect().width() );
}

void WaveletView::layoutBlips()
{
    layoutBlips( frameRect().width() );
}

void WaveletView::layoutBlips(qreal width)
{
    QGraphicsItem* focus = m_scene->focusItem();

//    m_lastWidth = width;
    qreal dy = 0;
    qreal dx = 0;
    foreach( Blip* blip, wavelet()->rootBlips() )
    {
        layoutBlip(blip, dx, dy, width);
    }

    setSceneRect( 0, 0, frameRect().width(), dy );

    if ( focus )
        m_scene->setFocusItem(focus);
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

//QGraphicsScene* WaveletView::headScene()
//{
//    return ((WaveView*)parent())->headScene();
//}

//void WaveletView::fitToWidth( qreal headWidth, qreal width )
//{
//    m_gfx->setWidth(headWidth);
//    layoutBlips(width);
//}

void WaveletView::setTitle( const QString& title )
{
    ((WaveView*)parent())->setTitle(title);
}

void WaveletView::resizeEvent( QResizeEvent* )
{
//    setSceneRect( 0, 0, frameRect().width(), sceneRect().height() );
    layoutBlips();
}
