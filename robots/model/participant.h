#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QObject>
#include <QPixmap>

class Participant : public QObject
{
    Q_OBJECT
public:
    Participant(const QString& address);

    void setAddress(const QString& address) { m_address = address; }
    /**
      * Address in the form user@domain.
      */
    QString address() const { return this->m_address; }
    /**
      * Display name of the participant
      */
    QString name() const { return this->m_name; }
    /**
      * Image in the size 40x40 pixels.
      */
    QPixmap pixmap() const { return m_pixmap; }
    void setName( const QString& name ) { this->m_name = name; }
    void setPixmap( const QPixmap& pixmap );

private:
    QString m_name;
    QString m_address;
    QPixmap m_pixmap;

    static QPixmap* s_defaultPixmap;
};

#endif // PARTICIPANT_H
