#include "waveletgraphicsitem.h"
#include "waveview.h"
#include "waveletview.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "participantgraphicsitem.h"
#include "buttongraphicsitem.h"
#include "addparticipantdialog.h"
#include "model/otprocessor.h"
#include "blipgraphicsitem.h"
#include "graphicstextitem.h"

#include <QPainter>
#include <QBrush>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QGraphicsScene>
#include <QApplication>

WaveletGraphicsItem::WaveletGraphicsItem(WaveView* view)
       : m_wavelet(0), m_view(view)
{    
    m_addUserButton = new ButtonGraphicsItem( QPixmap("images/adduser.png"), this );
    connect( m_addUserButton, SIGNAL(clicked()), SLOT(showAddParticipantDialog()));

    m_boldButton = new ButtonGraphicsItem( QPixmap("images/bold.png"), this );
    m_italicButton = new ButtonGraphicsItem( QPixmap("images/italic.png"), this );
    m_underlineButton = new ButtonGraphicsItem( QPixmap("images/underline.png"), this );
    m_strikeoutButton = new ButtonGraphicsItem( QPixmap("images/strikeout.png"), this );

    connect( m_boldButton, SIGNAL(clicked()), SLOT(boldClicked()));
    connect( m_italicButton, SIGNAL(clicked()), SLOT(italicCicked()));
    connect( m_underlineButton, SIGNAL(clicked()), SLOT(underlineClicked()));
    connect( m_strikeoutButton, SIGNAL(clicked()), SLOT(strikeoutClicked()));

    int dy = 42 + 2 * 5 + 1 + 22 + 1;
    m_rect = QRectF( 0, 0, 100, dy);

    setWavelet(view->wave()->wavelet());
}

void WaveletGraphicsItem::setWavelet( Wavelet* wavelet )
{
    if ( m_wavelet )
    {
        disconnect(wavelet, SIGNAL(participantAdded(Participant*)), this, SLOT(addParticipant(Participant*)));
        disconnect(wavelet, SIGNAL(participantRemoved(Participant*)), this, SLOT(removeParticipant(Participant*)));
    }
    m_wavelet = wavelet;
    connect(wavelet, SIGNAL(participantAdded(Participant*)), SLOT(addParticipant(Participant*)));
    connect(wavelet, SIGNAL(participantRemoved(Participant*)), SLOT(removeParticipant(Participant*)));

    updateParticipants();
}

void WaveletGraphicsItem::addParticipant(Participant*)
{
    updateParticipants();
}

void WaveletGraphicsItem::removeParticipant(Participant*)
{
    updateParticipants();
}

void WaveletGraphicsItem::updateParticipants()
{
    foreach( ParticipantGraphicsItem* item, m_participantItems)
    {
        delete item;
    }
    m_participantItems.clear();

    qreal dx = 0;
    qreal dy = 0;
    foreach( Participant* p, m_wavelet->participants() )
    {
        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(p, 42, false, this);
        item->setPos(dx + 5, dy + 5);
        dx += item->boundingRect().width() + 5;
        m_participantItems.append( item );
    }

    m_addUserButton->setPos( (42 + 5) * m_participantItems.count() + 5, dy + 16);
    m_boldButton->setPos( 10, 42 + 2 * 5 + 3 );
    m_italicButton->setPos( 10 + 20, 42 + 2 * 5 + 3 );
    m_underlineButton->setPos( 10 + 40, 42 + 2 * 5 + 3 );
    m_strikeoutButton->setPos( 10 + 60, 42 + 2 * 5 + 3 );
}

void WaveletGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    qreal dy = 0;

    QBrush brush2(QColor(0xc9,0xe2,0xfc));
    painter->fillRect(0, dy, m_rect.width(), 42 + 2 * 5, brush2);
    dy += 42 + 2 * 5;

    painter->setPen(QPen(QColor(0xaa,0xaa,0xaa)));
    painter->drawLine(0, dy, m_rect.width(), dy);
    painter->drawLine(0, dy + 23, m_rect.width(), dy + 23);

    QLinearGradient g(0, dy+1, 0, dy+23);
    g.setColorAt(0, QColor(255,255,255));
    g.setColorAt(1, QColor(0xdd,0xdd,0xdd));
    QBrush brush3(g);
    painter->fillRect(0, dy + 1, m_rect.width(), 22, brush3);
    dy += 1 + 22 + 1;
}

QRectF WaveletGraphicsItem::boundingRect() const
{
    return m_rect;
}

void WaveletGraphicsItem::setWidth(qreal width)
{
    m_rect.setWidth(width);
    this->prepareGeometryChange();
}

void WaveletGraphicsItem::showAddParticipantDialog()
{
    AddParticipantDialog dlg(m_wavelet->environment(), QApplication::activeWindow() );
    dlg.exec();

    if ( dlg.result() )
        m_wavelet->processor()->handleSendAddParticipant(dlg.result());
}

void WaveletGraphicsItem::boldClicked()
{
    BlipGraphicsItem* item = m_view->focusBlipItem();
    if ( !item )
        return;
    item->toggleBold();
    qDebug("Toggle bold");
    m_view->waveletView()->setFocus();
    // item->scene()->setFocusItem( item->textItem() );
}

void WaveletGraphicsItem::italicCicked()
{
}

void WaveletGraphicsItem::underlineClicked()
{
}

void WaveletGraphicsItem::strikeoutClicked()
{
}

