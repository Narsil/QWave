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
#include "insertgadgetdialog.h"
#include "bigbar.h"
#include "graphicstextitem.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QPixmap>
#include <QUrl>

WaveView::WaveView(Wave* wave, QWidget* parent )
        : QWidget(parent), m_wave(wave)
{
    setMinimumWidth(500);

    m_titleBar = new TitleBar(this);
    m_bigBar = new BigBar(this);
    m_toolBar = new ToolBar(this);

    m_gfx = new WaveletGraphicsItem(this);
    m_gfx->setPos(0,0);
    m_gfx->setZValue(1);
    m_bigBar->scene()->addItem(m_gfx);

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setSpacing(0);
    m_verticalLayout->setMargin(0);
    m_verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    m_verticalLayout->addWidget(m_titleBar);

    m_verticalLayout->addWidget(m_bigBar);
    m_verticalLayout->addWidget(m_toolBar);

    if ( wave )
        m_waveletView = new WaveletView(this, wave->wavelet());
    else
        m_waveletView = new WaveletView(this);
    m_waveletView->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Expanding );

    m_verticalLayout->addWidget(m_waveletView);

    m_boldButton = new ButtonGraphicsItem( QPixmap("images/bold.png"),0,0.5 );
    m_toolBar->addItem(m_boldButton);
    m_italicButton = new ButtonGraphicsItem( QPixmap("images/italic.png"),0,0.5 );
    m_toolBar->addItem(m_italicButton);
    m_underlineButton = new ButtonGraphicsItem( QPixmap("images/underline.png"),0,0.5 );
    m_toolBar->addItem(m_underlineButton);
    m_strikeoutButton = new ButtonGraphicsItem( QPixmap("images/strikeout.png"),0,0.5 );
    m_toolBar->addItem(m_strikeoutButton);
    m_imageButton = new ButtonGraphicsItem( QPixmap("images/image.png") );
    m_toolBar->addItem(m_imageButton);
    m_gadgetButton = new ButtonGraphicsItem( QPixmap("images/gadget.png") );
    m_toolBar->addItem(m_gadgetButton);

    connect( m_boldButton, SIGNAL(clicked()), SLOT(boldClicked()));
    connect( m_italicButton, SIGNAL(clicked()), SLOT(italicCicked()));
    connect( m_underlineButton, SIGNAL(clicked()), SLOT(underlineClicked()));
    connect( m_strikeoutButton, SIGNAL(clicked()), SLOT(strikeoutClicked()));
    connect( m_imageButton, SIGNAL(clicked()), SLOT(imageClicked()));
    connect( m_gadgetButton, SIGNAL(clicked()), SLOT(gadgetClicked()));
}

WaveView::~WaveView()
{
}

void WaveView::resizeEvent( QResizeEvent* )
{
    m_gfx->setWidth( m_bigBar->frameRect().width() );
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
    item->textItem()->toggleBold();
    m_waveletView->setFocus();
}

void WaveView::italicCicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;
    item->textItem()->toggleItalic();
    m_waveletView->setFocus();
}

void WaveView::underlineClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;
    item->textItem()->toggleUnderline();
    m_waveletView->setFocus();
}

void WaveView::strikeoutClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;
    item->textItem()->toggleStrikeout();
    m_waveletView->setFocus();
}

void WaveView::imageClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;

    InsertImageDialog dlg( m_wave->environment(), topLevelWidget() );
    if ( dlg.exec() == QDialog::Accepted )
    {
        item->textItem()->insertImage( dlg.url(), dlg.image(), dlg.thumbnail(), dlg.caption() );
    }
    m_waveletView->setFocus();
}

void WaveView::gadgetClicked()
{
    BlipGraphicsItem* item = focusBlipItem();
    if ( !item )
        return;

    InsertGadgetDialog dlg( m_wave->environment(), topLevelWidget() );
    if ( dlg.exec() == QDialog::Accepted )
    {
        item->textItem()->insertGadget( dlg.url() );
    }
    m_waveletView->setFocus();
}
