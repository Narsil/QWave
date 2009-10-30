#include "waveletgraphicsitem.h"
#include "waveletview.h"
#include "model/wavelet.h"
#include "participantgraphicsitem.h"
#include "buttongraphicsitem.h"
#include "addparticipantdialog.h"
#include "model/otprocessor.h"

#include <QPainter>
#include <QBrush>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QGraphicsScene>
#include <QApplication>

WaveletGraphicsItem::WaveletGraphicsItem(WaveletView* view)
       : m_wavelet(0), m_view(view)
{    
    m_addUserButton = new ButtonGraphicsItem( QPixmap("images/adduser.png"), this );
    connect( m_addUserButton, SIGNAL(clicked()), SLOT(showAddParticipantDialog()));

    int dy = 42 + 2 * 5 + 1 + 22 + 1;
    m_rect = QRectF( 0, 0, 100, dy);

    setWavelet(view->wavelet());
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

    m_addUserButton->setPos( (42 + 5) * m_participantItems.length() + 5, dy + 16);
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
