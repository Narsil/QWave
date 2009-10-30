#include "participantlistview.h"
#include "participantgraphicsitem.h"
#include "model/participant.h"

#include <QGraphicsScene>

ParticipantListView::ParticipantListView(QWidget* parent)
        : QGraphicsView( parent ), m_selectable(false)
{
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);
    setLineWidth(0);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setAttribute(Qt::WA_Hover, true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scene = new QGraphicsScene(this);
    setScene( m_scene );

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void ParticipantListView::setSelectable(bool selectable)
{
    if ( m_selectable == selectable )
        return;
    m_selectable = selectable;
    foreach( ParticipantGraphicsItem* item, m_items.values() )
    {
        item->setSelectable(m_selectable);
    }
}

void ParticipantListView::setParticipants( const QList<Participant*>& participants )
{
    foreach( ParticipantGraphicsItem* item, m_items.values() )
    {
        delete item;
    }
    m_items.clear();

    foreach( Participant* p, participants )
    {
        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(p, 28, true);
        item->setWidth( frameRect().width() );
        connect( item, SIGNAL(clicked(Participant*)), SLOT(selectParticipant(Participant*)));
        m_scene->addItem(item);
        m_items[p] = item;
    }
    m_selectedItem = 0;
    updateLayout();
}

void ParticipantListView::addParticipant(Participant* participant)
{
    if ( !m_items.contains(participant) )
    {
        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(participant, 28, true);
        item->setWidth( frameRect().width() );
        connect( item, SIGNAL(clicked(Participant*)), SLOT(selectParticipant(Participant*)));
        m_scene->addItem(item);
        m_items[participant] = item;
    }
    updateLayout();
}

void ParticipantListView::removeParticipant(Participant* participant)
{
    ParticipantGraphicsItem* item = m_items[participant];
    if ( item )
    {
        if ( item == m_selectedItem )
            m_selectedItem = 0;
        delete item;
        m_items.remove(participant);
        updateLayout();
    }
}

void ParticipantListView::updateLayout()
{
    int dy = 4;
    foreach(ParticipantGraphicsItem* item, m_items.values() )
    {
        item->setPos(8, dy);
        dy += item->boundingRect().height() + 4;
    }
}

void ParticipantListView::selectParticipant( Participant* participant )
{
    ParticipantGraphicsItem* item = m_items[participant];
    if ( !item )
        return;
    if ( m_selectedItem )
        m_selectedItem->setSelected(false);
    item->setSelected(true);
    m_selectedItem = item;
    emit participantSelected(participant);
}

void ParticipantListView::resizeEvent( QResizeEvent* )
{
    foreach(ParticipantGraphicsItem* item, m_items.values() )
    {
        item->setWidth( frameRect().width() );
    }
}
