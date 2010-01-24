#ifndef PARTICIPANTLISTVIEW_H
#define PARTICIPANTLISTVIEW_H

#include <QGraphicsView>
#include <QList>
#include <QHash>
#include <QString>

class QGraphicsScene;
class Participant;
class ParticipantGraphicsItem;

class ParticipantListView : public QGraphicsView
{
    Q_OBJECT
public:
    ParticipantListView(QWidget* parent);

    void setParticipants( const QList<Participant*>& participants );
    void addParticipant(Participant* participant);
    void removeParticipant(Participant* participant);

    void setSelectable(bool selectable);
    bool isSelectable() const { return m_selectable; }

public slots:
    void selectParticipant(Participant* participant);
    void setFilter( const QString& filter );

signals:
    void participantSelected(Participant* participant);

protected:
    void resizeEvent( QResizeEvent* event );

private:
    void updateLayout();

    QGraphicsScene* m_scene;
    QList<Participant*> m_participants;
    QHash<Participant*,ParticipantGraphicsItem*> m_items;
    ParticipantGraphicsItem* m_selectedItem;
    bool m_selectable;
    QString m_filter;
};

#endif // PARTICIPANTLISTVIEW_H
