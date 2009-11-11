#include "waveview.h"
#include "waveletview.h"
#include "model/wave.h"
#include "network/networkadapter.h"
#include "app/environment.h"
#include "titlebar.h"
#include "toolbar.h"
#include "waveletgraphicsitem.h"
#include "buttongraphicsitem.h"
#include "blipgraphicsitem.h"
#include "insertimagedialog.h"
#include "bigbar.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QPixmap>

WaveView::WaveView(Wave* wave, QWidget* parent )
        : QWidget(parent), m_wave(wave)
{
    setMinimumWidth(500);

    m_titleBar = new TitleBar(this);
    m_bigBar = new BigBar(this);
    m_toolBar = new ToolBar(this);

//    m_scene = new QGraphicsScene();
//    m_headScene = new QGraphicsScene();
    m_gfx = new WaveletGraphicsItem(this);
    m_gfx->setPos(0,0);
    m_gfx->setZValue(1);
    m_bigBar->scene()->addItem(m_gfx);
//    m_headScene->addItem(m_gfx);


    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_verticalLayout->addWidget(m_titleBar);

//    m_graphicsViewHead = new QGraphicsView(this);
//    m_graphicsViewHead->setObjectName(QString::fromUtf8("graphicsViewHead"));
//    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//    sizePolicy.setHorizontalStretch(0);
//    sizePolicy.setVerticalStretch(0);
//    sizePolicy.setHeightForWidth(m_graphicsViewHead->sizePolicy().hasHeightForWidth());
//    m_graphicsViewHead->setSizePolicy(sizePolicy);
//    m_graphicsViewHead->setMinimumSize(QSize(0, 76));
//    m_graphicsViewHead->setMaximumSize(QSize(16777215, 76));
//    m_graphicsViewHead->setFrameShape(QFrame::NoFrame);
//    m_graphicsViewHead->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_graphicsViewHead->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_graphicsViewHead->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//    m_verticalLayout->addWidget(m_graphicsViewHead);

    m_verticalLayout->addWidget(m_bigBar);
    m_verticalLayout->addWidget(m_toolBar);

    m_waveletView = new WaveletView(this, wave->wavelet());

    m_verticalLayout->addWidget(m_waveletView);

//    m_graphicsViewHead->setScene( m_headScene );
//    m_graphicsViewHead->setSceneRect( 0, 0, m_graphicsViewHead->frameRect().width(), m_headScene->itemsBoundingRect().height());

    m_boldButton = new ButtonGraphicsItem( QPixmap("images/bold.png") );
    m_toolBar->addItem(m_boldButton);
    m_italicButton = new ButtonGraphicsItem( QPixmap("images/italic.png") );
    m_toolBar->addItem(m_italicButton);
    m_underlineButton = new ButtonGraphicsItem( QPixmap("images/underline.png") );
    m_toolBar->addItem(m_underlineButton);
    m_strikeoutButton = new ButtonGraphicsItem( QPixmap("images/strikeout.png") );
    m_toolBar->addItem(m_strikeoutButton);
    m_imageButton = new ButtonGraphicsItem( QPixmap("images/image.png") );
    m_toolBar->addItem(m_imageButton);

    connect( m_boldButton, SIGNAL(clicked()), SLOT(boldClicked()));
    connect( m_italicButton, SIGNAL(clicked()), SLOT(italicCicked()));
    connect( m_underlineButton, SIGNAL(clicked()), SLOT(underlineClicked()));
    connect( m_strikeoutButton, SIGNAL(clicked()), SLOT(strikeoutClicked()));
    connect( m_imageButton, SIGNAL(clicked()), SLOT(imageClicked()));
}

WaveView::~WaveView()
{
//    delete m_scene;
//    delete m_headScene;
}

void WaveView::resizeEvent( QResizeEvent* )
{
    m_gfx->setWidth( m_bigBar->frameRect().width() );
//    m_graphicsViewHead->setSceneRect( 0, 0, m_graphicsViewHead->frameRect().width(), m_headScene->itemsBoundingRect().height());
}

void WaveView::setWave( Wave* wave )
{
    m_wave = wave;
    wave->environment()->networkAdapter()->openWavelet( wave->wavelet() );

    m_waveletView->setWavelet(wave->wavelet());    
    m_gfx->setWavelet(wave->wavelet());
}

void WaveView::setTitle( const QString& title )
{
    m_titleBar->setText(title);
}

BlipGraphicsItem* WaveView::focusBlipItem() const
{
    return m_waveletView->focusBlipItem();
}

void WaveView::boldClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;
    item->toggleBold();
    m_waveletView->setFocus();
}

void WaveView::italicCicked()
{
}

void WaveView::underlineClicked()
{
}

void WaveView::strikeoutClicked()
{
}

void WaveView::imageClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;

    InsertImageDialog dlg( m_wave->environment(), topLevelWidget() );
    if ( dlg.exec() == QDialog::Accepted )
    {
        item->insertImage( dlg.url(), dlg.image(), dlg.thumbnail(), dlg.caption() );
    }
    m_waveletView->setFocus();
}
