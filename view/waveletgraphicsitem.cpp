#include "waveletgraphicsitem.h"
#include "waveview.h"
#include "waveletview.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "participantgraphicsitem.h"
#include "buttongraphicsitem.h"
#include "addparticipantdialog.h"
#include "model/otprocessor.h"
#include "blipgraphicsitem.h"
#include "graphicstextitem.h"
#include "insertimagedialog.h"
#include "participantinfodialog.h"

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

    int dy = 42 + 2 * 5;
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
        connect(item,SIGNAL(clicked(Participant*)),SLOT(showParticipantInfo(Participant*)));
        m_participantItems.append( item );
    }

    m_addUserButton->setPos( (42 + 5) * m_participantItems.count() + 5, dy + 16);
}

void WaveletGraphicsItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
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

void WaveletGraphicsItem::showParticipantInfo(Participant* participant){
	ParticipantInfoDialog dlg(participant,m_view);
	connect( &dlg, SIGNAL(newWave(Participant*)),m_view, SIGNAL(newWave(Participant*)));
	dlg.exec();
}
