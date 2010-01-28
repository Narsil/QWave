#ifndef JID_H
#define JID_H

#include <QString>

class JID
{
public:
    JID();
    JID( const QString& jid );
    JID( const QString& name, const QString& domain );

    QString toString() const;
    QString name() const { return m_name; }
    QString domain() const { return m_domain; }
    bool isValid() const { return !m_name.isEmpty() || !m_domain.isEmpty(); }
    bool isNull() const { return !m_name.isEmpty() && !m_domain.isEmpty(); }

private:
    QString m_name;
    QString m_domain;
};

#endif // JID_H
