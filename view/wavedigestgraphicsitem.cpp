#include "wavedigestgraphicsitem.h"
#include "participantgraphicsitem.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/wavedigest.h"
#include "model/participant.h"

#include <QPen>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QDateTime>
#include <QDate>
#include <QTime>

WaveDigestGraphicsItem::WaveDigestGraphicsItem(Wave* wave, int width, QGraphicsItem* parent)
        : QObject(), QGraphicsItem(parent), m_wave(wave), m_rect(0 ,0, width, 38), m_hover(false)
{    
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    connect(wave->digest(), SIGNAL(digestChanged()), SLOT(updateDigest()));
    connect(wave->digest(), SIGNAL(participantAdded(Participant*)), SLOT(addParticipant(Participant*)));
    connect(wave->digest(), SIGNAL(participantRemoved(Participant*)), SLOT(removeParticipant(Participant*)));
    connect(wave, SIGNAL(dateChanged()), SLOT(repaint()));
    connect(wave, SIGNAL(blipCountChanged()), SLOT(repaint()));

    m_blipCount = m_wave->blipCount();
    m_unreadBlipCount = m_wave->unreadBlipCount();

    foreach( Participant* p, wave->wavelet()->participants() )
    {
        addParticipant(p);
    }

    m_textItem = new QGraphicsTextItem(this);
    m_textItem->setPos( 4 + 32 * 3, 1 );
    m_textItem->setTextWidth(width - m_textItem->x() - 4 - 70);

    QString digest = wave->digest()->toPlainText();
    if ( digest.isEmpty() )
        digest = "-";
    m_textItem->setPlainText( digest );
}

void WaveDigestGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    if ( isSelected() )
    {
        painter->fillRect( boundingRect(), QBrush(QColor(0xe0, 0xe8, 0xa4)));
        painter->setPen(QColor(0xce,0xd6,0x97));
        painter->drawLine(0, m_rect.height() - 1, m_rect.width() - 1, m_rect.height() - 1);
    }
    else if ( m_hover )
    {
        painter->fillRect( boundingRect(), QBrush(QColor(0xee,0xee,0xee)));
    }
    else
    {
        painter->setPen(QColor(0xee,0xee,0xee));
        painter->drawLine(0, m_rect.height() - 1, m_rect.width() - 1, m_rect.height() - 1);
    }

    // TODO: Get font from settings
    QFont font( "Arial", 10 );
    if ( m_unreadBlipCount > 0 )
        font.setBold( QFont::Bold );
    painter->setFont( font );
    QDateTime dt = m_wave->lastChange();
    QString date;
    if ( dt.date() == QDate::currentDate() )
        date = dt.time().toString("h:mm ap");
    else
        date = dt.date().toString("MMM d");
    painter->setPen( Qt::black );
    painter->drawText( m_rect.width() - 70, 14, date );

    if ( m_unreadBlipCount > 0 )
    {
        QString msgs = QString("%1").arg(m_unreadBlipCount);
        QFont font2( "Arial", 9, QFont::Bold );
        QFontMetrics metrics(font2);
        QRect rect = metrics.boundingRect(msgs);
        painter->setPen( Qt::NoPen );
        QBrush brush( QColor(0x99,0xbb,0x0) );
        painter->setBrush( brush );
        painter->drawRoundedRect( m_rect.width() - 70, 20, rect.width() + 12, 15, 6, 6 );
        painter->setFont(font2);
        painter->setPen( Qt::white );
        painter->drawText( m_rect.width() - 70 + 5, 32, msgs );
        QFont font3( "Arial", 10, QFont::Normal );
        painter->setFont(font3);
        painter->setPen( QColor(0x7f,0x7f,0x7f) );
        msgs = QString(tr("of %1")).arg(m_blipCount);
        painter->drawText( m_rect.width() - 70 + rect.width() + 15, 32, msgs );
    }
    else if ( m_blipCount > 0 )
    {
        painter->setPen( QColor(0x7f,0x7f,0x7f) );
        QFont font2( "Arial", 10, QFont::Normal );
        painter->setFont(font2);
        QString msgs = QString(tr("%1 msgs")).arg(m_blipCount);
        painter->drawText( m_rect.width() - 70, 32, msgs );
    }
}

QRectF WaveDigestGraphicsItem::boundingRect() const
{
    return m_rect;
}

void WaveDigestGraphicsItem::setWidth( int width )
{
    m_rect = QRectF(0 ,0, width, 38);
    m_textItem->setTextWidth(width - m_textItem->x() - 4 - 70);
    this->prepareGeometryChange();
}

void WaveDigestGraphicsItem::updateDigest()
{
    QString digest = m_wave->digest()->toPlainText();
    if ( digest.isEmpty() )
        digest = "-";
    m_textItem->setPlainText( digest );
}

void WaveDigestGraphicsItem::addParticipant(Participant* participant)
{
    if ( m_participants.count() >= 3 )
        return;
    ParticipantGraphicsItem* item = new ParticipantGraphicsItem(participant, 28, false, this);
    item->setPos( 4 + 32 * m_participants.count(), 4 );
    m_participants.append(item);
}

void WaveDigestGraphicsItem::removeParticipant(Participant* participant)
{
    bool isshown = false;
    foreach( ParticipantGraphicsItem* item, m_participants )
    {
        if ( item->participant() == participant )
            isshown = true;
    }
    if ( !isshown )
        return;

    foreach( ParticipantGraphicsItem* item2, m_participants )
    {
        delete item2;
    }
    m_participants.clear();

    for( int i = 0; i < qMin(3, m_wave->wavelet()->participants().count()); ++i )
    {
        addParticipant( m_wave->wavelet()->participants()[i] );
    }
}

void WaveDigestGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = true;
    update();
}

void WaveDigestGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    m_hover = false;
    update();
}

void WaveDigestGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent * )
{
    emit clicked(this);
}

void WaveDigestGraphicsItem::repaint()
{
    m_blipCount = m_wave->blipCount();
    m_unreadBlipCount = m_wave->unreadBlipCount();
    update();
}
