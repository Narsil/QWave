#include "waveletview.h"
#include "waveview.h"
#include "model/wavelet.h"
#include "blipgraphicsitem.h"
#include "model/blip.h"
#include "model/blipthread.h"
#include "graphicstextitem.h"

#include <QGraphicsScene>

WaveletView::WaveletView( WaveView* parent, Wavelet* wavelet )
        : QGraphicsView( parent ), m_wavelet(wavelet), m_focusItem(0)
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
    m_focusItem = 0;
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

    setSceneRect( 0, 0, frameRect().width(), dy + 10 );

    if ( focus )
        m_scene->setFocusItem(focus);
}

void WaveletView::layoutBlip(Blip* blip, qreal& xoffset, qreal& yoffset, qreal width )
{
    BlipGraphicsItem* b = m_blipItems[blip->id()];
    if ( !b )
    {
        b = new BlipGraphicsItem( blip, xoffset, yoffset, width - xoffset );
        bool ok = connect( b, SIGNAL(focusIn()), SLOT(focusIn()));
        Q_ASSERT(ok);
        ok = connect( b, SIGNAL(sizeChanged()), SLOT(layoutBlips()));
        Q_ASSERT(ok);
        ok = connect( b, SIGNAL(titleChanged(const QString&)), SLOT(setTitle(const QString&)));
        Q_ASSERT(ok);
        scene()->addItem(b);
        m_blipItems[blip->id()] = b;
    }
    else
    {
        b->setWidth(width - xoffset);
        b->setPos( xoffset, yoffset );
    }
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

void WaveletView::setTitle( const QString& title )
{
    ((WaveView*)parent())->setTitle(title);
}

void WaveletView::resizeEvent( QResizeEvent* )
{
    layoutBlips();
}

BlipGraphicsItem* WaveletView::focusBlipItem() const
{
    return m_focusItem;
}

void WaveletView::focusIn()
{
    Q_ASSERT( qobject_cast<BlipGraphicsItem*>(sender()) != 0 );
    m_focusItem = qobject_cast<BlipGraphicsItem*>(sender());
}
