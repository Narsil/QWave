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

//    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void ParticipantListView::setSelectable(bool selectable)
{
    if ( m_selectable == selectable )
        return;
    m_selectable = selectable;
    if ( m_selectable )
    {
        foreach( ParticipantGraphicsItem* item, m_items.values() )
        {
            item->setSelectable(m_selectable);
        }
    }
    else
    {
        // TODO
    }
}

void ParticipantListView::setParticipants( const QList<Participant*>& participants )
{
//    foreach( ParticipantGraphicsItem* item, m_items.values() )
//    {
//        delete item;
//    }
//    m_items.clear();

    m_participants = participants;
//    foreach( Participant* p, participants )
//    {
//        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(p, 28, true);
//        item->setWidth( frameRect().width() );
//        item->setSelectable(m_selectable);
//        connect( item, SIGNAL(clicked(Participant*)), SLOT(selectParticipant(Participant*)));
//        m_scene->addItem(item);
//        m_items[p] = item;
//    }
    m_selectedItem = 0;
    updateLayout();
}

void ParticipantListView::addParticipant(Participant* participant)
{
    if ( !m_participants.contains(participant) )
        m_participants.append( participant );

//    if ( !m_items.contains(participant) )
//    {
//        ParticipantGraphicsItem* item = new ParticipantGraphicsItem(participant, 28, true);
//        item->setWidth( frameRect().width() );
//        item->setSelectable(m_selectable);
//        connect( item, SIGNAL(clicked(Participant*)), SLOT(selectParticipant(Participant*)));
//        m_scene->addItem(item);
//        m_items[participant] = item;
//    }
    updateLayout();
}

void ParticipantListView::removeParticipant(Participant* participant)
{
    m_participants.removeAll(participant);

//    ParticipantGraphicsItem* item = m_items[participant];
//    if ( item )
//    {
//        if ( item == m_selectedItem )
//            m_selectedItem = 0;
//        delete item;
//        m_items.remove(participant);
//        updateLayout();
//    }

    updateLayout();
}

//void ParticipantListView::updateLayout()
//{
//    int dy = 4;
//    foreach(ParticipantGraphicsItem* item, m_items.values() )
//    {
//        item->setPos(8, dy);
//        dy += item->boundingRect().height() + 4;
//    }
//
//    setSceneRect( 0, 0, frameRect().width(), dy );
//}

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

    setSceneRect( 0, 0, frameRect().width(), sceneRect().height() );
}

void ParticipantListView::setFilter( const QString& filter )
{
    m_filter = filter;
    updateLayout();
}

void ParticipantListView::updateLayout()
{
    QHash<Participant*,ParticipantGraphicsItem*> map;

    int dy = 4;
    foreach( Participant* p, m_participants )
    {
        if ( m_filter.length() == 0 || ( p->name().length() >= m_filter.length() && p->name().left(m_filter.length()).toLower() == m_filter.toLower() ) )
        {
            ParticipantGraphicsItem* item = m_items[p];
            if ( !item )
            {
                item = new ParticipantGraphicsItem(p, 28, true);
                item->setWidth( frameRect().width() );
                item->setSelectable(m_selectable);
                connect( item, SIGNAL(clicked(Participant*)), SLOT(selectParticipant(Participant*)));
                m_scene->addItem(item);
            }            
            map[p] = item;
            item->setPos(8, dy);
            dy += item->boundingRect().height() + 4;
        }
    }

    foreach( Participant* a, m_items.keys() )
    {
        if ( !map.contains(a) )
        {
            ParticipantGraphicsItem* item = m_items[a];
            if ( item == m_selectedItem )
                m_selectedItem = 0;
            delete item;
        }
    }
    m_items = map;

    setSceneRect( 0, 0, frameRect().width(), dy );
}
