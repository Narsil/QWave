#ifndef CONTACTS_H
#define CONTACTS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QList>

class Participant;
class Environment;

class Contacts : public QObject
{
    Q_OBJECT
public:
    Contacts(Environment* environment, QObject* parent = 0);

    Participant* participant( const QString& address ) const;
    Participant* addParticipant( const QString& address );
    QList<Participant*> participants() const;

signals:
    void participantAdded(Participant* participant);
    void participantRemoved(Participant* participant);

private:
    QHash<QString,Participant*> m_participants;
    Environment* m_environment;
};

#endif // CONTACTS_H
