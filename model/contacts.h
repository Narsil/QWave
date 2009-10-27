#ifndef CONTACTS_H
#define CONTACTS_H

#include <QObject>
#include <QHash>
#include <QString>

class Participant;
class Environment;

class Contacts : public QObject
{
public:
    Contacts(Environment* environment, QObject* parent = 0);

    Participant* participant( const QString& address ) const;
    Participant* addParticipant( const QString& address );

private:
    QHash<QString,Participant*> m_participants;
    Environment* m_environment;
};

#endif // CONTACTS_H
