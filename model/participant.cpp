#include "participant.h"

#include <QPixmap>

QPixmap* Participant::s_defaultPixmap = 0;

Participant::Participant(const QString& address)
        : m_name(address), m_address(address)
{
    if ( !s_defaultPixmap )
        s_defaultPixmap = new QPixmap("images/unknown.gif");
    m_pixmap = *s_defaultPixmap;
}

void Participant::setPixmap( const QPixmap& pixmap )
{
    m_pixmap = pixmap;
}

