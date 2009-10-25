#include "waveview.h"
#include "waveletview.h"
#include "model/wave.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>

WaveView::WaveView(Wave* wave )
        : m_wave(wave)
{
    setMinimumWidth(500);

    m_scene = new QGraphicsScene();
    m_headScene = new QGraphicsScene();

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_graphicsViewHead = new QGraphicsView(this);
    m_graphicsViewHead->setObjectName(QString::fromUtf8("graphicsViewHead"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_graphicsViewHead->sizePolicy().hasHeightForWidth());
    m_graphicsViewHead->setSizePolicy(sizePolicy);
    m_graphicsViewHead->setMinimumSize(QSize(0, 98));
    m_graphicsViewHead->setMaximumSize(QSize(16777215, 98));
    m_graphicsViewHead->setFrameShape(QFrame::NoFrame);
    m_graphicsViewHead->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsViewHead->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsViewHead->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_verticalLayout->addWidget(m_graphicsViewHead);

    m_graphicsView = new QGraphicsView(this);
    m_graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
    m_graphicsView->setFrameShape(QFrame::NoFrame);
    m_graphicsView->setFrameShadow(QFrame::Plain);
    m_graphicsView->setLineWidth(0);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setInteractive(true);
    m_graphicsView->setAttribute(Qt::WA_Hover, true);
    m_graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_verticalLayout->addWidget(m_graphicsView);

    m_waveletView = new WaveletView(this, wave->wavelet());

    m_graphicsView->setScene( m_scene );
    m_graphicsView->setSceneRect(0,0,1000,2000);
    m_graphicsViewHead->setScene( m_headScene );
    m_graphicsViewHead->setSceneRect( m_headScene->itemsBoundingRect());
}

WaveView::~WaveView()
{
    delete m_scene;
    delete m_headScene;
}

void WaveView::resizeEvent( QResizeEvent* )
{
    m_waveletView->fitToWidth( m_graphicsViewHead->frameRect().width(), m_graphicsView->frameRect().width() );
}
